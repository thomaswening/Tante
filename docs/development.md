# Tante: Development

How work on Tante is done and checked. Sections on building, testing and CI are added with the first code.

## Commits and pull requests

Every commit follows [Conventional Commits](https://www.conventionalcommits.org/) (`type(scope): summary`), so the changelog can be generated. Nothing is committed to `main` directly: every change, document or code, goes through a pull request that Thomas reviews and merges.

## Writing documentation

Decided by Thomas, 25 October 2026.

- **Two layers.** Project records (requirements, [ADRs](glossary.md#adr), [spikes](glossary.md#spike), architecture, roadmap) live in `docs/`. User documentation will live in `docs/user/`, sorted into tutorials, how-to guides, reference and explanation ([Diátaxis](https://diataxis.fr/)). Each subfolder is created with its first document.
- **Terms.** A term of art is linked to the [glossary](glossary.md) on its first use in each document, e.g. `[limb](glossary.md#limb)`. A new term gets a glossary entry in the same pull request that first uses it. <!-- glossary-ignore: term -->
- **Mathematics in LaTeX.** Formulas are written in LaTeX, which GitHub renders: inline as `$R_{\mu\nu} = 0$`, displayed between `$$` lines. Plain text is kept for code and for FORM or Tante input.
- **Language.** British English, sentence-case headings, absolute dates ("21 October 2026"), short declarative sentences.
- **Links.** Requirements, open decisions, ADRs and other documents are referred to by links, never by bare IDs or paths.
- **Diagrams.** Architecture, data layouts and workflows get a Mermaid diagram. Measured results get a chart.

## Tools and checks

### Weekly external-link check

Every Monday at 06:00 UTC, [docs-weekly.yml](../.github/workflows/docs-weekly.yml) runs `tools/check_docs.py --main` on `main`. Then [lychee](https://github.com/lycheeverse/lychee) follows every external link in `README.md` and `docs/`. A failure blocks nothing. It opens the issue "Weekly docs check failed", or comments on the one still open. <!-- glossary-ignore: block -->

**Rule: every external link answers with a success status within 60 seconds, unless [lychee.toml](../lychee.toml) excludes it.** Cited pages move or vanish without anyone changing the repository. Some sites refuse automated requests although the page is there. Those are excluded by pattern in `lychee.toml`, each with the reason. Decided by Thomas, 9 November 2026.

| Link | Result | Why |
|---|---|---|
| a page that answers 200, also after a redirect | accepted | |
| a page that answers 404, or a host that stays unreachable | reported | the citation is broken |
| `https://doi.org/10.1103/…` (APS) | excluded | the publisher answers 403 to automated requests |
| `https://github.com/…` | accepted | the workflow passes a token, so GitHub's limit for anonymous requests does not apply |
| a slow host that answers within 60 seconds | accepted | three retries |
| `https://gmplib.org/�`, `https://www.nikhef.nl/�` | excluded | the host never answers GitHub's runners, although it answers a browser |

**When it fails.** Open each reported link in a browser.

- The page is gone: link its new address. If there is none, link an archived copy from the [Wayback Machine](https://web.archive.org/) and say so in the citation, as for Ruijl's 2023 slides in [prior art](prior-art.md).
- The page opens in a browser but lychee gets 403 every week: add a pattern for that site to `lychee.toml`, with the reason and the date it was checked.
- The host times out week after week but opens in a browser within seconds: it drops requests from GitHub's runners. Add a pattern to `lychee.toml` as for a 403.
- The failure is a one-off timeout or 5xx: wait for the next week. Excluding it would hide a real breakage later.

Close the issue once the run is clean. The run can be started by hand: Actions → docs-weekly → Run workflow.

**Running it locally** needs [lychee](https://github.com/lycheeverse/lychee#installation):

```bash
lychee --no-progress "README.md" "docs/**/*.md"
```
