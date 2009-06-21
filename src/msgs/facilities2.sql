/* MAX_NUMBER is the next number to be used, always one more than the highest message number. */
set bulk_insert INSERT INTO FACILITIES (LAST_CHANGE, FACILITY, FAC_CODE, MAX_NUMBER) VALUES (?, ?, ?, ?);
--
('2009-06-18 08:22:56', 'JRD', 0, 661)
('2005-09-02 00:55:59', 'QLI', 1, 513)
('2008-11-28 20:27:04', 'GDEF', 2, 346)
('2008-11-28 20:09:22', 'GFIX', 3, 120)
('1996-11-07 13:39:40', 'GPRE', 4, 1)
--
--('1996-11-07 13:39:40', 'GLTJ', 5, 1)
--('1996-11-07 13:39:40', 'GRST', 6, 1)
--
('2005-11-05 13:09:00', 'DSQL', 7, 32)
('2008-11-13 17:07:03', 'DYN', 8, 256)
--
--('1996-11-07 13:39:40', 'FRED', 9, 1)
--
('1996-11-07 13:39:40', 'INSTALL', 10, 1)
('1996-11-07 13:38:41', 'TEST', 11, 4)
('2009-05-13 11:12:07', 'GBAK', 12, 317)
('2009-06-05 23:07:00', 'SQLERR', 13, 970)
('1996-11-07 13:38:42', 'SQLWARN', 14, 613)
('2006-09-10 03:04:31', 'JRD_BUGCHK', 15, 307)
--
--('1996-11-07 13:38:43', 'GJRN', 16, 241)
--
('2008-11-28 20:59:39', 'ISQL', 17, 165)
('2009-02-12 21:16:16', 'GSEC', 18, 101)
('2002-03-05 02:30:12', 'LICENSE', 19, 60)
('2002-03-05 02:31:54', 'DOS', 20, 74)
('2007-04-07 13:11:00', 'GSTAT', 21, 38)
('2008-09-16 16:51:42', 'FBSVCMGR', 22, 51)
('2007-12-21 19:03:07', 'UTL', 23, 2)
stop

COMMIT WORK;
