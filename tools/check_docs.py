# check_docs.py 1.2 (claude-config project-docs)
"""Check a project's documents and code for drift: links, anchors, IDs, ADRs, diagrams, glossary.

    check_docs.py [repo-root] [--source GLOB ...] [--tests GLOB ...] [--repo-url URL]
                  [--strict-links] [--require-coverage] [--base REF] [--main] [--no-mermaid]
                  [--glossary-all]

The master copy lives in claude-config (skills/project-docs/scripts/check_docs.py). Projects
copy it into tools/ and keep the version line above, so a stale copy can be recognised.

Requirements (when docs/requirements.md exists):
  * every requirement definition has an anchor matching its id and a level;
  * every OPEN definition has an anchor matching its id; no id is defined twice;
  * REQ-nnn / OPEN-x / ADR-nnnn mentions in docs/ are defined, and are links (warning only).
Links (every Markdown file under docs/, and README.md):
  * relative links point at existing files;
  * a #fragment names an anchor of the target: a GitHub heading anchor or an <a id="..."> anchor.
ADRs (when docs/adr/ exists):
  * every record's Status matches its row in docs/adr/README.md; with --main, none is Proposed;
  * a record whose title names an OPEN-x links it, and the OPEN entry links back to the record.
Mermaid: every ```mermaid or ~~~mermaid block parses (needs Node's npx; skipped locally without it, an error in CI).
Roadmap (when docs/roadmap.md exists): every open OPEN entry's Needed by note names a roadmap phase,
  and each phase's Needs line lists exactly the OPENs that name it.
With --base REF (a pull request's base branch):
  * if docs/requirements.md changed, its Version line and revision history changed too;
  * glossary coverage (when docs/glossary.md exists), in changed files only: the first use of a
    glossary term is a link to its entry; all-caps identifiers and bold in-line definitions are in
    the glossary or in docs/glossary-ignore.txt. <!-- glossary-ignore: word --> exempts a word in
    one paragraph or list item. A term's bold definition in a document its entry's Defined in line
    links is its first use there, so it needs no link. --glossary-all checks every document instead.
With --source and --tests, scans code files (globs relative to the root, ** allowed):
  * every REQ/OPEN/ADR id mentioned exists;
  * every link into docs/ - relative (docs/requirements.md#req-012, ../docs/...) or absolute
    under --repo-url (https://github.com/o/r/blob/main/docs/...) - points at an existing file
    and anchor;
  * in source files, a mention on a comment line with no link to its definition is a warning,
    or an error with --strict-links. Test tags are ids and need no link.
With --require-coverage, fails when a SHALL requirement is referenced by no test and not listed
in docs/manual-verification.md.
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

REQ_DEF = re.compile(
    r'(?:<a id="(?P<anchor>[a-z0-9-]+)"></a>)?\*\*(?P<id>REQ-\d{3}) — (?P<title>[^(*]+?)'
    r'\((?P<level>SHALL|SHALL NOT|SHOULD|SHOULD NOT|COULD|MAY|OUT OF SCOPE)\)\.\*\*')
OPEN_DEF = re.compile(r'(?:<a id="(?P<anchor>[a-z0-9-]+)"></a>)?\*\*(?P<id>OPEN-[A-Z]{1,2}) — ')
LOOSE_DEF = re.compile(r"\*\*(?P<id>REQ-\d{3}|OPEN-[A-Z]{1,2}) — ")
ANCHOR = re.compile(r'<a id="([a-z0-9-]+)"></a>')
HTML_ID = re.compile(r'<[a-zA-Z][^>]*?\b(?:id|name)="([^"]+)"')
LINK = re.compile(r"\[([^\]]*)\]\(<?([^)\s>]+)>?(?:\s+\"[^\"]*\")?\)")
REF_LINK = re.compile(r"^\s{0,3}\[[^\]]+\]:\s*<?(\S+?)>?(?:\s+\"[^\"]*\")?\s*$", re.M)
HEADING = re.compile(r"^(#{1,6})\s+(.*?)\s*#*\s*$")
FENCE = re.compile(r"^\s*(```|~~~)")
MENTION = re.compile(r"\b(REQ-\d{3}|OPEN-[A-Z]{1,2}|ADR-\d{4})\b")
OPEN_ID = re.compile(r"\bOPEN-[A-Z]{1,2}\b")
REQ_ID = re.compile(r"\bREQ-\d{3}\b")
# A path into docs/ as it appears in code: relative, or the tail of a repository URL.
DOC_LINK = re.compile(r"(?<![\w-])(?:\.\./)*(?P<path>docs/[\w./-]+?\.md)(?:#(?P<anchor>[\w-]+))?(?![\w/.-])")
COMMENT_LINE = re.compile(r"^\s*(?://|/\*|\*|#|--|;|%|\"\"\"|''')")
ADR_STATUS = re.compile(r"^\s*[-*]?\s*\*\*Status:?\*\*:?\s*(.+)$", re.M)
ADR_STATES = ("proposed", "accepted", "superseded", "deprecated", "rejected", "withdrawn")
# Absolute links into the repository at these refs are checked locally; others (a commit, a spike
# branch) are left to the weekly external-link run.
LOCAL_REFS = ("main", "master", "HEAD")
# <!-- glossary-ignore: term, other term --> exempts those words in one paragraph or list item:
# the one the comment stands in, or the next one when the comment has a line of its own.
DOC_IGNORE = re.compile(r"<!--\s*glossary-ignore:\s*(.*?)\s*-->")
# a paragraph ends at a blank line or where a list item starts
BLOCK_BREAK = re.compile(r"\n[ \t]*\n|\n(?=[ \t]*(?:[-*+]|\d+\.)\s)")
MERMAID_CLI ="@mermaid-js/mermaid-cli@11.4.2"
# Words that look like identifiers but are the requirement keywords themselves.
BUILTIN_IGNORE = {"SHALL", "SHOULD", "COULD", "MAY", "MUST", "NOT", "OUT", "OF", "SCOPE", "WHEN",
                  "THEN", "IF", "BEFORE", "WHILE", "AFTER", "WHERE", "ONLY", "AND", "OR", "TODO"}


def read(path):
    return path.read_text(encoding="utf-8", errors="replace")


def strip_code(text, fill=" "):
    """Blank out fenced blocks and inline code (with fill), keeping line numbers and offsets."""
    out, fenced = [], False
    for line in text.splitlines():
        if FENCE.match(line):
            fenced = not fenced
            out.append("")
        elif fenced:
            out.append("")
        else:
            out.append(re.sub(r"`[^`]*`", lambda m: fill * len(m[0]), line))
    return "\n".join(out)


def strip_links_and_code(text):
    text = strip_code(text)
    text = LINK.sub("", text)
    text = ANCHOR.sub("", text)
    text = LOOSE_DEF.sub("", text)  # a definition is not a mention
    text = re.sub(r"^# ADR-\d{4}.*$", "", text, flags=re.M)  # an ADR's own title
    return text


def github_slug(heading):
    """The anchor GitHub gives a heading: rendered text, lower case, punctuation dropped, spaces to hyphens."""
    text = LINK.sub(lambda m: m[1], heading)
    text = re.sub(r"<[^>]+>", "", text)
    text = text.strip().lower()
    text = re.sub(r"[^\w\- ]", "", text)
    return text.replace(" ", "-")


_anchor_cache = {}


def anchors_of(path):
    """Every anchor a link may name in a Markdown file: heading anchors and HTML id/name attributes."""
    path = path.resolve()
    if path not in _anchor_cache:
        result, seen = set(), {}
        code_free = strip_code(read(path))
        for line in code_free.splitlines():
            m = HEADING.match(line)
            if m:
                slug = github_slug(m[2])
                n = seen.get(slug, 0)
                seen[slug] = n + 1
                result.add(slug if n == 0 else f"{slug}-{n}")
        result.update(HTML_ID.findall(code_free))
        _anchor_cache[path] = result
    return _anchor_cache[path]


def links_in(text):
    """(line number, target) of every inline and reference link outside code and HTML comments."""
    code_free = re.sub(r"<!--.*?-->", lambda m: re.sub(r"[^\n]", " ", m[0]), strip_code(text), flags=re.S)
    for n, line in enumerate(code_free.splitlines(), 1):
        for _, target in LINK.findall(line):
            yield n, target
    for m in REF_LINK.finditer(code_free):
        yield code_free.count("\n", 0, m.start()) + 1, m[1]


def check_links(md, root, repo_url, errors):
    rel = md.relative_to(root).as_posix()
    for n, target in links_in(read(md)):
        tail = target[len(repo_url.rstrip("/") + "/blob/"):].split("/", 1) if repo_url else []
        if repo_url and target.startswith(repo_url.rstrip("/") + "/blob/") and len(tail) == 2                 and tail[0] in LOCAL_REFS:
            # an absolute link into this repository's main line: check it against the checkout
            dest_path, _, frag = tail[1].partition("#")
            dest = (root / dest_path).resolve()
        elif re.match(r"^[a-zA-Z][a-zA-Z0-9+.-]*:", target):
            continue  # external: the weekly lychee run checks those
        else:
            path, _, frag = target.partition("#")
            dest = (md.parent / path).resolve() if path else md.resolve()
        if not dest.exists():
            errors.append(f"{rel}:{n}: link to missing file {target}")
        elif frag and dest.suffix == ".md" and frag not in anchors_of(dest):
            errors.append(f"{rel}:{n}: link to missing anchor {target}")


def definition_target(mention, adrs):
    """The link target that counts as linking a mention: an anchor or an ADR file."""
    if mention.startswith("ADR-"):
        return adrs.get(mention)
    return "requirements.md#" + mention.lower()


def check_code_file(path, rel, text, root, ids, adrs, repo_url, is_source, errors, warnings, strict):
    if repo_url:
        text_for_links = re.sub(re.escape(repo_url.rstrip("/")) + r"/blob/[^/\s]+/", "", text)
    else:
        text_for_links = text
    for n, line in enumerate(text_for_links.splitlines(), 1):
        for m in DOC_LINK.finditer(line):
            dest = (root / m["path"]).resolve()
            if not dest.exists():
                errors.append(f"{rel}:{n}: link to missing file {m['path']}")
            elif m["anchor"] and m["anchor"] not in anchors_of(dest):
                errors.append(f"{rel}:{n}: link to missing anchor {m['path']}#{m['anchor']}")
        for mention in MENTION.findall(line):
            if mention.startswith("ADR-"):
                if mention not in adrs:
                    errors.append(f"{rel}:{n}: names {mention}, which has no record in docs/adr/")
                    continue
            elif mention not in ids:
                errors.append(f"{rel}:{n}: names {mention}, which is not defined")
                continue
            if is_source and COMMENT_LINE.match(line) and definition_target(mention, adrs) not in line:
                (errors if strict else warnings).append(f"{rel}:{n}: unlinked mention of {mention}")


def open_entries(req_text):
    """OPEN id -> the text of its entry (from its definition to the next definition or heading)."""
    starts = [(m.start(), m["id"]) for m in OPEN_DEF.finditer(req_text)]
    entries = {}
    for i, (start, oid) in enumerate(starts):
        end = starts[i + 1][0] if i + 1 < len(starts) else len(req_text)
        heading = re.search(r"^#{1,6} ", req_text[start:end], re.M)
        entries[oid] = req_text[start:start + heading.start()] if heading else req_text[start:end]
    return entries


def adr_state(text):
    word = text.strip().split()[0].strip("*:.,;()").lower() if text.strip() else ""
    return word if word in ADR_STATES else None


def check_adrs(docs, adrs, opens, main_branch, errors):
    adr_dir = docs / "adr"
    index = adr_dir / "README.md"
    rows = {}
    if index.exists():
        for n, line in enumerate(read(index).splitlines(), 1):
            if not line.startswith("|"):
                continue
            m = re.search(r"\]\(\.?/?(\d{4}-[^)#]+\.md)\)", line)
            if m:
                cells = [c.strip() for c in line.strip().strip("|").split("|")]
                rows[m[1]] = (n, cells[-1])
    for adr_id, name in sorted(adrs.items()):
        path = adr_dir / name
        text = read(path)
        m = ADR_STATUS.search(text)
        file_state = adr_state(m[1]) if m else None
        if file_state is None:
            errors.append(f"docs/adr/{name}: no recognisable Status line (- **Status:** Accepted ...)")
        if index.exists():
            if name not in rows:
                errors.append(f"docs/adr/README.md: no row for {adr_id} ({name})")
            else:
                n, cell = rows[name]
                row_state = adr_state(cell)
                if file_state and row_state != file_state:
                    errors.append(f"docs/adr/README.md:{n}: {adr_id} is '{cell}' in the index "
                                  f"but {file_state.capitalize()} in its record")
        if main_branch and file_state == "proposed":
            errors.append(f"docs/adr/{name}: {adr_id} is Proposed on main (merging should accept it)")
        title = next((l for l in text.splitlines() if l.startswith("# ")), "")
        for oid in sorted(set(OPEN_ID.findall(title))):
            if f"requirements.md#{oid.lower()}" not in text:
                errors.append(f"docs/adr/{name}: title names {oid} but the record does not link "
                              f"../requirements.md#{oid.lower()}")
            if oid not in opens:
                errors.append(f"docs/adr/{name}: title names {oid}, which is not defined")
            elif f"adr/{name}" not in opens[oid]:
                errors.append(f"docs/requirements.md: {oid} does not link back to adr/{name}, "
                              f"which settles it")
    for name, (n, _) in rows.items():
        if not (adr_dir / name).exists():
            errors.append(f"docs/adr/README.md:{n}: row for missing record {name}")


def is_open(entry):
    """True unless the entry's last Resolved/Recorded/Withdrawn marker is not followed by 'still open'."""
    marks = list(re.finditer(r"\*\*(Resolved|Recorded|Withdrawn)\*\*", entry))
    if not marks:
        return True
    return bool(re.search(r"\b(still open|remains open)\b", entry[marks[-1].end():], re.I))


def check_roadmap(docs, opens, errors):
    roadmap = docs / "roadmap.md"
    text = strip_code(read(roadmap))
    phases, current = {}, None  # anchor -> (number, line of Needs, set of OPENs)
    for n, line in enumerate(text.splitlines(), 1):
        h = HEADING.match(line)
        if h:
            m = re.match(r"Phase\s+(\w+)", h[2])
            current = github_slug(h[2]) if m else None
            if m:
                phases[current] = [m[1], None, set()]
        elif current and re.match(r"^\s*[-*]\s*\*{0,2}Needs:?\*{0,2}:?", line):
            phases[current][1] = n
            phases[current][2] = set(OPEN_ID.findall(line))
    if not phases:
        errors.append("docs/roadmap.md: no phase headings ('### Phase N: Name') found")
        return
    named = {anchor: set() for anchor in phases}
    unplaced = set()  # open, but rule A already reported its note: missing or naming no phase
    by_number = {v[0]: anchor for anchor, v in phases.items()}
    for oid, entry in sorted(opens.items()):
        if not is_open(entry):
            continue
        m = re.search(r"\*\*Needed by:?\*\*:?(.*?)(?=\*\*[A-Z(]|\Z)", entry, re.S)
        if not m:
            errors.append(f"docs/requirements.md: {oid} is open but has no **Needed by:** note")
            unplaced.add(oid)
            continue
        note = m[1]
        targets = {a for a in re.findall(r"roadmap\.md#([\w-]+)", note) if a in phases}
        targets |= {by_number[x] for x in re.findall(r"\bPhase\s+(\w+)", note) if x in by_number}
        if not targets:
            errors.append(f"docs/requirements.md: {oid}'s Needed by note names no roadmap phase "
                          f"('Phase N' or a link to roadmap.md#phase-...)")
            unplaced.add(oid)
        for a in targets:
            named[a].add(oid)
    for anchor, (number, line, needs) in phases.items():
        where = f"docs/roadmap.md:{line}" if line else "docs/roadmap.md"
        if line is None and named[anchor]:
            errors.append(f"docs/roadmap.md: Phase {number} has no Needs line "
                          f"but {', '.join(sorted(named[anchor]))} name it")
            continue
        for oid in sorted(named[anchor] - needs):
            errors.append(f"{where}: Phase {number}'s Needs line lacks {oid}, whose Needed by names it")
        for oid in sorted(needs - named[anchor]):
            if oid in opens and is_open(opens[oid]) and oid not in unplaced:
                errors.append(f"{where}: Phase {number} needs {oid}, whose Needed by note names another phase")


def check_mermaid(md_files, root, errors, notes):
    blocks = []
    for md in md_files:
        lines = read(md).splitlines()
        i = 0
        while i < len(lines):
            opening = re.match(r"^\s*(```|~~~)\s*mermaid\s*$", lines[i])
            if opening:
                start, body = i + 1, []
                i += 1
                while i < len(lines) and not lines[i].lstrip().startswith(opening[1]):
                    body.append(lines[i])
                    i += 1
                blocks.append((md, start, "\n".join(body)))
            i += 1
    if not blocks:
        return
    npx = shutil.which("npx")
    if not npx:
        msg = f"{len(blocks)} Mermaid diagrams not checked: Node's npx was not found"
        (errors if os.environ.get("CI") else notes).append(msg)
        return
    with tempfile.TemporaryDirectory() as tmp:
        tmp = Path(tmp)
        config = tmp / "puppeteer.json"
        config.write_text('{"args": ["--no-sandbox"]}', encoding="utf-8")
        for k, (md, line, body) in enumerate(blocks):
            src = tmp / f"d{k}.mmd"
            src.write_text(body, encoding="utf-8")
            run = subprocess.run([npx, "--yes", MERMAID_CLI, "--quiet", "-p", str(config),
                                  "-i", str(src), "-o", str(tmp / f"d{k}.svg")],
                                 capture_output=True, text=True, encoding="utf-8", errors="replace")
            if run.returncode != 0:
                detail = next((l for l in (run.stderr + run.stdout).splitlines()
                               if "error" in l.lower() or "expecting" in l.lower()), "").strip()
                errors.append(f"{md.relative_to(root).as_posix()}:{line}: Mermaid diagram does not parse"
                              + (f": {detail[:200]}" if detail else ""))


def git(root, *args):
    run = subprocess.run(["git", "-C", str(root), *args], capture_output=True, text=True,
                         encoding="utf-8", errors="replace")
    if run.returncode != 0:
        raise RuntimeError(f"git {' '.join(args)}: {run.stderr.strip()}")
    return run.stdout


def changed_since(root, base):
    """(merge base, set of repo-relative paths changed on this branch, committed or not)."""
    mb = git(root, "merge-base", base, "HEAD").strip()
    names = set(git(root, "diff", "--name-only", mb).split("\n"))
    names |= set(git(root, "ls-files", "--others", "--exclude-standard").split("\n"))
    return mb, {n for n in names if n}


def check_requirements_version(root, mb, errors):
    try:
        old = git(root, "show", f"{mb}:docs/requirements.md")
    except RuntimeError:
        return  # new file
    new = read(root / "docs" / "requirements.md")

    def version(text):
        m = re.search(r"^Version:.*$", text, re.M)
        return m[0] if m else None

    def history(text):
        m = re.search(r"^#{1,6} .*Revision history.*$", text, re.M | re.I)
        if not m:
            return None
        return [l for l in text[m.end():].splitlines() if l.startswith("|")]

    if old == new:
        return
    if version(old) == version(new):
        errors.append("docs/requirements.md changed but its Version line did not")
    if history(old) == history(new):
        errors.append("docs/requirements.md changed but its revision history has no new or changed row")


def glossary_entries(glossary):
    """anchor -> (term, names, defining documents).

    Entry format: <a id="x"></a>**term** (alias, other alias) *(kind)*: ... Each comma-separated
    alias is one name. The defining documents are the files the entry's Defined in: line links.
    """
    text = read(glossary)
    starts = list(re.finditer(r'<a id="([a-z0-9-]+)"></a>\*\*([^*]+)\*\*([^:\n]*?)\*\(', text))
    entries = {}
    for i, m in enumerate(starts):
        term, extra = m[2].strip(), m[3].strip()
        names = {term}
        alias = re.fullmatch(r"\((.*)\)", extra)
        if alias:
            names |= {a.strip(" `") for a in alias[1].split(",") if a.strip(" `…")}
        body = text[m.end():starts[i + 1].start() if i + 1 < len(starts) else len(text)]
        defining = set()
        for line in re.findall(r"^\s*[-*]\s*Defined in:(.*)$", body, re.M):
            for _, target in LINK.findall(line):
                path = target.partition("#")[0]
                if path and not re.match(r"^[a-zA-Z][a-zA-Z0-9+.-]*:", path):
                    defining.add((glossary.parent / path).resolve())
        entries[m[1]] = (term, names, defining)
    return entries


def bare(name):
    """A name as candidates and glossary names are compared: lower case, no leading article."""
    return re.sub(r"^(the|a|an)\s+", "", name.strip().lower())


def term_pattern(term):
    words = [re.escape(w) for w in term.split()]
    if len(words) > 1 and words[0].lower() in ("the", "a", "an"):
        words[0] = "(?i:" + words[0] + ")"  # "The FORM corpus" opens a sentence
    body = r"\s+".join(words)
    flags = 0 if any(c.isupper() for c in term) else re.I
    return re.compile(r"(?<![\w-])" + body + r"(?:s|es)?(?![\w-])", flags)


# a bold span: one ** ... ** pair on one line, scanned left to right so pairs never overlap
BOLD = re.compile(r"\*\*([^\n]+?)\*\*")
# what follows a bold in-line definition: an optional *(kind)*, then = or :
DEF_TAIL = re.compile(r"\s*(?:\*\([^)]*\)\*\s*)?(?::|=)")
# what may stand before a bold term that opens a paragraph, list item or sentence
# (searched in the text from the start of that paragraph or list item)
DEF_LEAD = re.compile(r"^[ \t]*(?:(?:[-*+]|\d+\.)[ \t]+)?(?:(?:the|a|an)\s+)?$"
                      r"|[.!?]\s+(?:(?:the|a|an)\s+)?$", re.I)


def block_start(prose, pos):
    """Offset where the paragraph or list item that holds pos begins."""
    return max((b.end() for b in BLOCK_BREAK.finditer(prose, 0, pos)), default=0)


def definition_spans(prose, term):
    """(start, end) of each bold span in prose that defines term: **term** =, **term**:,
    **term** is/are, **Term at each step:**, or **term** opening a paragraph, list item or
    sentence (an article before it allowed)."""
    pat = term_pattern(term)
    spans = []
    for b in BOLD.finditer(prose):
        inner = b[1].strip()
        head = re.match(r"(?:(?:the|a|an)\s+)?", inner, re.I)
        if not pat.match(inner, head.end()) and not pat.match(inner):
            continue
        after = prose[b.end():b.end() + 40]
        if (DEF_TAIL.match(after) or re.match(r"\s+(?:is|are)\b", after) or inner.endswith(":")
                or DEF_LEAD.search(prose[block_start(prose, b.start()):b.start()])):
            spans.append((b.start(), b.end()))
    return spans


def load_ignore(path):
    if not path.exists():
        return set()
    words = set()
    for line in read(path).splitlines():
        word = line.split("#", 1)[0].strip()
        if word:
            words.add(word.lower())
    return words


def exempt_spans(prose):
    """(start, end, words) for each <!-- glossary-ignore: ... -->: the paragraph or list item it
    stands in, or the next one when the comment has a line of its own."""
    spans = []
    for m in DOC_IGNORE.finditer(prose):
        start = block_start(prose, m.start())
        text = re.search(r"\S", prose[m.end():])
        pos = m.end() + text.start() if text else len(prose)
        blank = BLOCK_BREAK.search(prose, pos)
        words = {w.strip().lower() for w in m[1].split(",") if w.strip()}
        spans.append((start, blank.start() if blank else len(prose), words))
    return spans


def check_glossary(md_files, root, docs, errors):
    glossary = docs / "glossary.md"
    entries = glossary_entries(glossary)
    ignore = load_ignore(docs / "glossary-ignore.txt") | {w.lower() for w in BUILTIN_IGNORE}
    known = {bare(n) for _, names, _ in entries.values() for n in names}
    # the phrases whose first use must be a link: each term and its multi-word aliases.
    # Longest first, so "module interface" is not also counted as a use of "module".
    phrases = sorted({(n, anchor) for anchor, (term, names, _) in entries.items()
                      for n in names if n == term or len(n.split()) > 1},
                     key=lambda p: (-len(p[0]), p))
    for md in md_files:
        if md.resolve() == glossary.resolve():
            continue
        rel = md.relative_to(root).as_posix()
        text = read(md)
        # prose only: headings are never the place a term is introduced
        prose = "\n".join("" if HEADING.match(l) else l for l in strip_code(text).splitlines())
        # the same, with inline code marked rather than blanked, to tell bold code from bold words
        coded = "\n".join("" if HEADING.match(l) else l for l in strip_code(text, "\x01").splitlines())
        spans = exempt_spans(prose)

        def exempt(word, pos):
            return any(start <= pos < end and word.lower() in words for start, end, words in spans)

        linked = {}  # anchor -> offset of the first link to it
        for m in LINK.finditer(prose):
            path, _, frag = m[2].partition("#")
            if frag and path and (md.parent / path).resolve() == glossary.resolve():
                linked.setdefault(frag, m.start())
        no_links = LINK.sub(lambda m: "\0" * len(m[0]), prose)  # same offsets, link text hidden
        no_links = ANCHOR.sub(lambda m: " " * len(m[0]), no_links)
        no_links = DOC_IGNORE.sub(lambda m: " " * len(m[0]), no_links)
        masked = no_links
        first = {}  # anchor -> (offset, end, phrase) of its first use
        for phrase, anchor in phrases:
            pat = term_pattern(phrase)
            # a glossary term is never exempt for a whole document: only in a marked paragraph
            m = next((m for m in pat.finditer(masked) if not exempt(phrase, m.start())), None)
            if m and (anchor not in first or m.start() < first[anchor][0]):
                first[anchor] = (m.start(), m.end(), phrase)
            masked = pat.sub(lambda m: "\0" * len(m[0]), masked)
        for anchor, (pos, end, phrase) in sorted(first.items(), key=lambda kv: kv[1][0]):
            # linked before, or right after in brackets: "GNU General Public License ([GPL](...))"
            if anchor in linked and (linked[anchor] <= pos
                                     or re.fullmatch(r"\s*\(?\s*", prose[end:linked[anchor]])):
                continue
            term, names, defining = entries[anchor]
            # the term's own source definition is its first use there: linking back would be circular
            if md.resolve() in defining and any(
                    start <= pos < end for n in names for start, end in definition_spans(prose, n)):
                continue
            n = prose.count("\n", 0, pos) + 1
            shown = f"'{term}'" if phrase == term else f"'{phrase}' (an alias of '{term}')"
            errors.append(f"{rel}:{n}: first use of {shown} is not linked to "
                          f"the glossary (glossary.md#{anchor})")
        candidates = {}
        for m in re.finditer(r"(?<![\w.-])([A-Z][A-Z0-9_]{2,})(?![\w-])", no_links):
            if not exempt(m[1], m.start()):
                candidates.setdefault(m[1], m.start())
        # bold in-line definitions: short noun phrases only, never a sentence or bold code
        for b in BOLD.finditer(coded):
            word = b[1].strip()
            if (not DEF_TAIL.match(coded, b.end()) or len(word.split()) > 4
                    or re.search(r"[,.:\x01]", word)):
                continue
            if not exempt(word, b.start()):
                candidates.setdefault(word, b.start())
        for word, pos in candidates.items():
            if MENTION.fullmatch(word) or word in ("REQ", "OPEN"):
                continue
            if sum(ch.isalpha() for ch in word) < 2:
                continue
            lw = bare(word)
            if lw in known or lw.rstrip("s") in known or lw in ignore or word.lower() in ignore:
                continue
            n = prose.count("\n", 0, pos) + 1
            errors.append(f"{rel}:{n}: '{word}' looks like a term but is not in the glossary "
                          f"(add an entry, or a line to docs/glossary-ignore.txt)")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("root", nargs="?", default=".")
    ap.add_argument("--source", nargs="*", default=[], help="globs of source files, relative to root")
    ap.add_argument("--tests", nargs="*", default=[], help="globs of test files, relative to root")
    ap.add_argument("--repo-url", help="e.g. https://github.com/owner/repo; links under it are checked locally")
    ap.add_argument("--strict-links", action="store_true", help="unlinked mentions in source comments are errors")
    ap.add_argument("--require-coverage", action="store_true")
    ap.add_argument("--base", help="git ref of the base branch; enables the checks of changed files")
    ap.add_argument("--main", action="store_true", help="checks for main: no ADR may be Proposed")
    ap.add_argument("--no-mermaid", action="store_true", help="skip the Mermaid check")
    ap.add_argument("--glossary-all", action="store_true",
                    help="check glossary coverage in every document, not only changed ones")
    args = ap.parse_args()
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

    root = Path(args.root)
    docs = root / "docs"
    req_path = docs / "requirements.md"
    errors, warnings, notes = [], [], []
    if not docs.is_dir():
        print(f"error: {docs} not found")
        return 2
    md_files = sorted(docs.rglob("*.md"))
    if (root / "README.md").exists():
        md_files.append(root / "README.md")

    adrs = {}
    adr_dir = docs / "adr"
    if adr_dir.exists():
        for f in adr_dir.glob("*.md"):
            m = re.match(r"(\d{4})-.+\.md$", f.name)
            if m:
                adrs["ADR-" + m[1]] = f.name

    ids, levels, opens = {}, {}, {}
    if req_path.exists():
        req_text = read(req_path)
        opens = open_entries(req_text)
        for m in REQ_DEF.finditer(req_text):
            rid = m["id"]
            if rid in ids:
                errors.append(f"{rid} defined twice")
            ids[rid] = m["anchor"]
            levels[rid] = m["level"]
            if m["anchor"] != rid.lower():
                errors.append(f"{rid}: anchor missing or wrong (expected <a id=\"{rid.lower()}\"></a>)")
        for m in OPEN_DEF.finditer(req_text):
            oid = m["id"]
            if oid in ids:
                errors.append(f"{oid} defined twice")
            ids[oid] = m["anchor"]
            if m["anchor"] != oid.lower():
                errors.append(f"{oid}: anchor missing or wrong (expected <a id=\"{oid.lower()}\"></a>)")
        for m in LOOSE_DEF.finditer(req_text):
            if m["id"] not in ids:
                errors.append(f"{m['id']}: looks like a definition but is malformed (anchor, ' — ', or '(LEVEL).**')")
    else:
        notes.append("no docs/requirements.md: requirement and OPEN checks skipped")

    for md in md_files:
        check_links(md, root, args.repo_url, errors)
        if not req_path.exists() or md.parent.name == "adr" and md.name in ("template.md", "README.md"):
            continue
        if docs.resolve() not in md.resolve().parents:
            continue  # the README is checked for links only
        rel = md.relative_to(root).as_posix()
        for mention in MENTION.findall(strip_links_and_code(read(md))):
            if mention.startswith(("REQ", "OPEN")) and mention not in ids:
                errors.append(f"{rel}: mentions {mention}, which is not defined")
            elif mention.startswith("ADR") and mention not in adrs:
                errors.append(f"{rel}: mentions {mention}, which has no record in docs/adr/")
            else:
                warnings.append(f"{rel}: unlinked mention of {mention}")

    if adrs:
        check_adrs(docs, adrs, opens, args.main, errors)
    if (docs / "roadmap.md").exists():
        check_roadmap(docs, opens, errors)
    if not args.no_mermaid:
        check_mermaid(md_files, root, errors, notes)

    if args.base:
        try:
            mb, changed = changed_since(root, args.base)
        except RuntimeError as e:
            print(f"error: {e}")
            return 2
        if req_path.exists() and "docs/requirements.md" in changed:
            check_requirements_version(root, mb, errors)
        if (docs / "glossary.md").exists():
            changed_md = [md for md in md_files
                          if args.glossary_all or md.relative_to(root).as_posix() in changed]
            check_glossary(changed_md, root, docs, errors)
        else:
            notes.append("no docs/glossary.md: glossary coverage skipped")
    elif args.glossary_all and (docs / "glossary.md").exists():
        check_glossary(md_files, root, docs, errors)
    else:
        notes.append("no --base: requirement-version and glossary checks skipped")

    tested = set()
    for globs, is_source in ((args.source, True), (args.tests, False)):
        for pattern in globs:
            for f in sorted(root.glob(pattern)):
                if not f.is_file():
                    continue
                text = read(f)
                if not is_source:
                    tested.update(REQ_ID.findall(text))
                check_code_file(f, f.relative_to(root).as_posix(), text, root, ids, adrs,
                                args.repo_url, is_source, errors, warnings, args.strict_links)

    if args.require_coverage:
        manual = docs / "manual-verification.md"
        manual_ids = set(REQ_ID.findall(read(manual))) if manual.exists() else set()
        for rid, level in sorted(levels.items()):
            if level == "SHALL" and rid not in tested and rid not in manual_ids:
                errors.append(f"{rid} (SHALL) is covered by no test and not in manual-verification.md")

    for note in notes:
        print(f"note: {note}")
    for w in sorted(set(warnings)):
        print(f"warning: {w}")
    for e in errors:
        print(f"error: {e}")
    n_req = sum(1 for k in ids if k.startswith("REQ"))
    print(f"{n_req} requirements, {len(ids) - n_req} open decisions, {len(adrs)} ADRs, "
          f"{len(md_files)} Markdown files, {len(errors)} errors, {len(set(warnings))} warnings")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
