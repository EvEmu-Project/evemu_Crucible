# EVEmu CI

CI is setup using GitLab currently; where we use a standardized build container image. This image contains everything needed to build EVEmu.

The reasoning for doing this is so that we do not have to re-install all dependencies every time a build is run. This image is public and available here:

quay.io/evemu/build-image:latest

The Dockerfile for this image is contained here.
