-- Tidy up SQL script for the psotnic !seen module written by Stuart Scott <stu@wilf.co.uk>
--  all it does it delete all entries older than 30 days, change it to whatever interval you want
--  then setup a cronjob calling 'mysql -u <user> --password=<pass> <db> < seen-tidy.sql'
DELETE FROM seen WHERE part < DATE_SUB(NOW(),INTERVAL 30 DAY)
