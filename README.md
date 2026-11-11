# Tante

Tante is a symbolic mathematics engine for physicists, from elementary algebra to calculations too long to do by hand. It is meant as a modern, well-documented successor to [FORM](https://github.com/form-dev/form): a new, independent implementation that is easier to understand, maintain and extend.

## Status

Phase 1 of the [roadmap](docs/roadmap.md#phase-1-foundations), Foundations: the build, CI on Windows, macOS and Linux, and `tante --version`. Nothing calculates yet. How to build and test: [Development](docs/development.md#building-and-testing). The project is specified in:

- [Requirements](docs/requirements.md): what Tante must do, verifiably
- [Prior-art survey](docs/prior-art.md): existing systems, their limits, and the known results Tante will be validated against
- [Architecture](docs/architecture.md): how Tante is built, its parts and where later capabilities enter
- [Architecture decision records](docs/adr/README.md): why Tante is built the way it is
- [Glossary](docs/glossary.md): the terms the documents use, from mathematics, engineering and Tante itself <!-- glossary-ignore: term -->
- [Development](docs/development.md): how work on Tante is done, including how its documents are written

## Credit

Tante owes its model of calculation to FORM, by Jos Vermaseren and the FORM developers (Nikhef; now developed at [github.com/form-dev/form](https://github.com/form-dev/form)). Compatibility will be measured against FORM 5.0.2. Tante is not a fork of FORM and contains none of its source code.

## Licence

[GNU General Public License](docs/glossary.md#gpl), version 3 or (at your option) any later version. See [LICENSE](LICENSE).
