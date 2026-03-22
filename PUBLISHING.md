# Publishing To GitHub

This project is set up so GitHub Actions can build macOS and Windows installers on GitHub-hosted runners.

## Recommended flow

1. Create a new public GitHub repository.
2. Push this project to that repository.
3. Push a version tag such as `v0.1.0`.
4. GitHub Actions will build the installers and publish a release bundle.

## Suggested commands

If this local repo is the one you want to publish:

```bash
git add .
git commit -m "Prepare public release pipeline"
git branch -M main
git remote add origin <your-github-repo-url>
git push -u origin main
git tag v0.1.0
git push origin v0.1.0
```

## What the workflow does

The workflow in `.github/workflows/build-installers.yml` will:

- build macOS AU, VST3, and standalone targets
- package the macOS installer `.pkg`
- build the Windows VST3 on a GitHub Windows runner
- package the Windows installer `.exe`
- upload both installers and a combined zip to the GitHub release for tagged builds

## Notes

- `workflow_dispatch` is also enabled, so you can run the pipeline manually from GitHub Actions.
- For tagged builds, use tags formatted like `v0.1.0`.
- Build outputs and release archives are ignored locally by `.gitignore`.
