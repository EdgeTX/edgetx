# Git Command Reference

## Quick checklist

### Terminal commands

	ls - lists all of the folders
	ls -la - lists all of the files
	cd .. - returns one dir back
	cd - enters a directory
	. - just install in the current directory

### On initial install

	git --version - checks the version of the installed locally git
	git config --global user.name "Your Name" - sets up the name of the user
	git config --global user.email "yourname@somemail.eu" - sets up the mail of the user
	git config --list - lists all the git configurations

### For help on commands

	git help <verb> (e.g. git help config) OR
	git <verb> --help

### For initializing the project

	git init - initializes the git repo in the current folder
	touch .gitignore - creates a git ignore file
	git status - check working tree - both on the git and on local

### Add files

	git add -A - adds all of the files for commiting
	remember - git status - to check the state of the repo

### Remove files

	git reset - removes files to be commited
	git reset somefile.js - removes somefile.js from the commit preparation

### Committing

	git commit -m "This is the commit message" - -m is used to add message

### Check log

	git log - renders commit ids, authors, dates

### Clone a remote repo

	git clone <url> <where to clone>

### View info about the repo

	git remote -v - lists info about the repo
	git branch -a - lists all of the branches

### View changes

	git diff - shows the difference made in the files

### Pull before push ALWAYS

	git pull origin master

### THEN PUSH

	git push origin master - <origin> name of remote repo <master> the branch that we push to

### First time push of the branch

	git push -u origin <name of the branch> - -u coordinates the two branches (local and on server)

### Create a branch

	git branch <name of the branch>

### Checkout a branch

	git checkout <name of the branch>

### Merge a branch

	git checkout master
	git pull origin master
	git branch --merged - see which branches are merged
	git merge <name of the branch you want to merge>
	git push origin master

### Delete a branch

	git branch -d <name of the branch> - this deletes it locally!!!
	git branch -a - check the repo branches
	git push origin --delete <name of the branch> - this deletes it from the repo!
