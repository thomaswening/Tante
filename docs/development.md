# Tante: Development

How work on Tante is done and checked. Sections on building, testing and CI are added with the first code.

## Commits and pull requests

Every commit follows [Conventional Commits](https://www.conventionalcommits.org/) (`type(scope): summary`), so the changelog can be generated. Nothing is committed to `main` directly: every change, document or code, goes through a pull request that Thomas reviews and merges.

## Writing documentation

Decided by Thomas, 25 October 2026.

- **Two layers.** Project records (requirements, ADRs, spikes, architecture, roadmap) live in `docs/`. User documentation will live in `docs/user/`, sorted into tutorials, how-to guides, reference and explanation ([Diátaxis](https://diataxis.fr/)). Each subfolder is created with its first document.
- **Terms.** A term of art is linked to the [glossary](glossary.md) on its first use in each document, e.g. `[limb](glossary.md#limb)`. A new term gets a glossary entry in the same pull request that first uses it.
- **Mathematics in LaTeX.** Formulas are written in LaTeX, which GitHub renders: inline as `$R_{\mu\nu} = 0$`, displayed between `$$` lines. Plain text is kept for code and for FORM or Tante input.
- **Language.** British English, sentence-case headings, absolute dates ("21 October 2026"), short declarative sentences.
- **Links.** Requirements, open decisions, ADRs and other documents are referred to by links, never by bare IDs or paths.
- **Diagrams.** Architecture, data layouts and workflows get a Mermaid diagram. Measured results get a chart.
