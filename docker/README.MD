# OpenKO Docker Scripts
### ⚠️⚠️ **DO NOT use this to run a database for a public server - do a manual installation instead.** ⚠️⚠️

## Docker Requirements
You must install Docker yourself and ensure it is available on the command line (try `docker version` in a terminal.)

Docker isn't for everyone.  It takes time to learn and troubleshoot issues.  If you're uncomfortable with the potential learning curve, you can check [SQL Server Manual Setup](https://github.com/Open-KO/KnightOnline/wiki/SQL-Server-2022-Manual-Setup) for manual database install instructions.

## Purpose: Local Development Database
These Docker services are only meant to be used for local development enviroments.
No precautions have been taken to make any of the containers secure.

⚠️ **DO NOT use this to run a database for a public server - do a manual installation instead.** ⚠️

## Configuration
If you feel compelled to change the default configuration: 
* Make a copy of the file `default.env` in the root project directory (KnightOnline/), and name the new copy `.env`. 
* Make sure `default.env` does not get deleted, as this is a reference file available for everyone.  If you push a PR with
this file missing the PR will be rejected until you restore the file.
* This `.env` file is on .gitignore, so you do not need to worry about accidentally
commiting your personal configuration.

Absent a local port conflict there should be no need to change default config. It is recommended
to just use the defaults.

## Scripts in this folder
This folder contains several helper scripts to help you create and manage a copy of the OpenKO database using Docker. Depending on your platform, you would want to use the script that ends in `.sh` for linux-based systems, and `.cmd` for Windows.

* `clean_setup`: Use this for a first-time setup. Removes any existing sqlserver/kodb-util images/volumes, then creates/starts new versions of them. Before the script completes, the script to create an up-to-date OpenKO database will be invoked.
* `reset_database`: Performs a clean import of the OpenKO database.  This will remove any existing OpenKO database instance matching the DB_NAME from the .env file (default: KN_online), then create a replacement database with the latest data from the OpenKO-db project.
* `stop_containers`: Stops the sqlserver and kodb-util services. 
* `resume_containers`: Resumes the sqlserver and kodb-util services.
* `uninstall`: Removes all sqlserver and kodb-util containers, images, and volumes.

Most IDEs will allow you to right click and run these scripts from a context menu; otherwise call them from your terminal
application.  Linux users may need to run `chmod +x *.sh` in the `KnightOnline/docker` folder to make the scripts executable.
