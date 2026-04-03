# EdgeTX Developer Documentation

This folder contains the source for the EdgeTX developer documentation site, built with [MkDocs](https://www.mkdocs.org/) using the [Material theme](https://squidfunk.github.io/mkdocs-material/).

## Prerequisites

Install the required packages from the repo root. Using [`uv`](https://docs.astral.sh/uv/getting-started/installation/) is recommended:

```bash
uv pip install -r docs-requirements.txt
```

Or with plain pip:

```bash
pip install -r docs-requirements.txt
```

[`mike`](https://github.com/jimporter/mike) is included as a dependency — it manages versioned deployments of the site (e.g. publishing a new release). You won't need it for local editing or preview.

## Local preview

Serve the docs locally with live reload:

```bash
mkdocs serve
```

Then open [http://127.0.0.1:8000](http://127.0.0.1:8000) in your browser. The site will automatically refresh as you edit files.

## Building

To build a static copy of the site into the `site/` directory:

```bash
mkdocs build
```

CI runs `mkdocs build --strict` on pull requests, which treats warnings (e.g. broken internal links) as errors. It's worth running this locally before submitting a PR to catch any issues early:

```bash
mkdocs build --strict
```

## Deployment

The docs are deployed automatically via GitHub Actions (`.github/workflows/docs.yml`):

- Pushes to `main` are deployed as the `latest` version.
- Release tags (e.g. `v2.11.0`) are deployed as a numbered version (e.g. `v2.11`) and aliased as `stable`.

## Project structure

```
docs/
├── assets/          # Images, stylesheets
├── building/        # Build guides (Windows, Linux, macOS, CodeSpaces)
├── contributing/    # Git workflow and contribution guides
├── development/     # Developer reference (CLI, protocols, etc.)
├── hardware/        # Radio specs and hardware reference
├── mods/            # Hardware modification guides
└── troubleshooting/ # Troubleshooting guides
```

Site navigation is defined in [`mkdocs.yml`](../mkdocs.yml) at the repo root.

## Adding or editing pages

1. Create or edit a `.md` file in the appropriate subfolder.
2. If adding a new page, add it to the `nav:` section in `mkdocs.yml`.
3. Preview with `mkdocs serve` and verify with `mkdocs build --strict` before submitting a PR.
