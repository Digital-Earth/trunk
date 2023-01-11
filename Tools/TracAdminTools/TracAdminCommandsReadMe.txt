about
	-- Shows information about trac-admin

help
	-- Show documentation

initenv
	-- Create and initialize a new environment interactively

initenv <projectname> <db> <repostype> <repospath> <templatepath>
	-- Create and initialize a new environment from arguments

hotcopy <backupdir>
	-- Make a hot backup copy of an environment

resync
	-- Re-synchronize trac with the repository

resync <rev>
	-- Re-synchronize only the given <rev>

upgrade
	-- Upgrade database to current version

wiki list
	-- List wiki pages

wiki remove <name>
	-- Remove wiki page

wiki export <page> [file]
	-- Export wiki page to file or stdout

wiki import <page> [file]
	-- Import wiki page from file or stdin

wiki dump <directory>
	-- Export all wiki pages to files named by title

wiki load <directory>
	-- Import all wiki pages from directory

wiki upgrade
	-- Upgrade default wiki pages to current version

permission list [user]
	-- List permission rules

permission add <user> <action> [action] [...]
	-- Add a new permission rule

permission remove <user> <action> [action] [...]
	-- Remove permission rule

component list
	-- Show available components

component add <name> <owner>
	-- Add a new component

component rename <name> <newname>
	-- Rename a component

component remove <name>
	-- Remove/uninstall component

component chown <name> <owner>
	-- Change component ownership

ticket remove <number>
	-- Remove ticket

ticket_type list
	-- Show possible ticket types

ticket_type add <value>
	-- Add a ticket type

ticket_type change <value> <newvalue>
	-- Change a ticket type

ticket_type remove <value>
	-- Remove a ticket type

ticket_type order <value> up|down
	-- Move a ticket type up or down in the list

priority list
	-- Show possible ticket priorities

priority add <value>
	-- Add a priority value option

priority change <value> <newvalue>
	-- Change a priority value

priority remove <value>
	-- Remove priority value

priority order <value> up|down
	-- Move a priority value up or down in the list

severity list
	-- Show possible ticket severities

severity add <value>
	-- Add a severity value option

severity change <value> <newvalue>
	-- Change a severity value

severity remove <value>
	-- Remove severity value

severity order <value> up|down
	-- Move a severity value up or down in the list

version list
	-- Show versions

version add <name> [time]
	-- Add version

version rename <name> <newname>
	-- Rename version

version time <name> <time>
	-- Set version date (Format: "YYYY-MM-DD" or "now")

version remove <name>
	-- Remove version

milestone list
	-- Show milestones

milestone add <name> [due]
	-- Add milestone

milestone rename <name> <newname>
	-- Rename milestone

milestone due <name> <due>
	-- Set milestone due date (Format: "YYYY-MM-DD" or "now")

milestone completed <name> <completed>
	-- Set milestone completed date (Format: "YYYY-MM-DD" or "now")

milestone remove <name>
	-- Remove milestone
