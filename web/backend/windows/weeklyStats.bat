@echo off
set logParser="C:\Program Files (x86)\Log Parser 2.2\LogParser.exe"
set logs='C:\inetpub\logs\LogFiles\W3SVC1\*.log'
set startTime=%~1
set startTime=%startTime:-=/%
set endTime=%~2
set endTime=%endTime:-=/%
set resultDir=%3
set commonOptions=-i:w3c -q:ON

echo %startTime% %endTime% %resultDir%

MKDIR "%resultDir%"

set top=100
set outFile=%resultDir%TopSearches.txt
%logParser% %commonOptions% "SELECT TOP %top% COUNT(*) AS [Requests], STRCAT(TO_STRING(MUL(PROPCOUNT(*), 100)), '%%') AS [Percentage], URLUNESCAPE(EXTRACT_VALUE(cs-uri-query, 'Search')) as Search INTO %outFile% FROM %logs% WHERE cs-method = 'GET' AND Search <> NULL AND (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) GROUP BY Search ORDER BY [Requests] DESC"
echo Finished %outFile%

set top=1000
set outFile=%resultDir%TopAGSAndOGCSearches.txt
%logParser% %commonOptions% "SELECT TOP %top% COUNT(*) AS [Requests], STRCAT(TO_STRING(MUL(PROPCOUNT(*), 100)), '%%') AS [Percentage], URLUNESCAPE(EXTRACT_VALUE(cs-uri-query, 'Search')) as Search INTO %outFile% FROM %logs% WHERE cs-method = 'GET' AND Search <> NULL AND Search LIKE '%%http%%' AND (Search LIKE '%%getcapabilities%%' OR Search LIKE '%%rest/services%%') AND (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) GROUP BY Search ORDER BY [Requests] DESC"
echo Finished %outFile%

set top=1000
set outFile=%resultDir%TopAGSAndOGCRawSearches.txt
%logParser% %commonOptions% "SELECT TOP %top% COUNT(*) AS [Requests], EXTRACT_VALUE(cs-uri-query, 'Search') as Search INTO %outFile% FROM %logs% WHERE cs-method = 'GET' AND Search <> NULL AND Search LIKE '%%http%%' AND (Search LIKE '%%getcapabilities%%' OR Search LIKE '%%rest/services%%') AND (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) GROUP BY Search ORDER BY [Requests] DESC"
echo Finished %outFile%

set top=100
set outFile=%resultDir%TopRequests.txt
%logParser% %commonOptions% "SELECT TOP %top% COUNT(*) AS [Requests], STRCAT(TO_STRING(MUL(PROPCOUNT(*), 100)), '%%') AS [Percentage], cs-method, cs-uri-stem, STRCAT(TO_STRING(MAX(time-taken)),'ms') AS MaximumTime, STRCAT(TO_STRING(AVG(time-taken)),'ms') AS AverageTime, STRCAT(TO_STRING(MIN(time-taken)),'ms') AS MinimumTime INTO %outFile% FROM %logs% WHERE (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) GROUP BY cs-method, cs-uri-stem ORDER BY [Requests] DESC"
echo Finished %outFile%

set outFile=%resultDir%TotalRequests.txt
%logParser% %commonOptions% "SELECT COUNT(*), STRCAT('(Average time of ',STRCAT(TO_STRING(AVG(time-taken)),'ms)')) AS AverageTime INTO %outFile% FROM %logs% WHERE (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss')))"
echo Finished %outFile%

set outFile=%resultDir%UserActivity.txt
%logParser% %commonOptions% "SELECT COUNT(*) AS [Requests], STRCAT(TO_STRING(MUL(PROPCOUNT(*), 100)), '%%') AS [Percentage], cs-username, STRCAT(TO_STRING(MAX(time-taken)),'ms') AS MaximumTime, STRCAT(TO_STRING(AVG(time-taken)),'ms') AS AverageTime, STRCAT(TO_STRING(MIN(time-taken)),'ms') AS MinimumTime INTO %outFile% FROM %logs% WHERE (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) GROUP BY cs-username ORDER BY [Requests] DESC"
echo Finished %outFile%

set tmpFile=%resultDir%DailyUserActivity.txt
%logParser% %commonOptions% -o:W3C "SELECT DISTINCT cs-username, date INTO %tmpFile% FROM %logs% WHERE (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) GROUP BY cs-username, date ORDER BY date ASC"
echo Finished %tmpFile%
set outFile=%resultDir%UsersPerDay.gif
%logParser% %commonOptions% "SELECT date, COUNT(cs-username) as UniqueVisitor INTO %outFile% FROM %tmpFile% GROUP BY date ORDER BY date ASC" -o:CHART -chartType:ColumnStacked -ChartTitle:"Users Per Day" 
echo Finished %outFile%

set outFile=%resultDir%FailedRequests.txt
%logParser% %commonOptions% "SELECT date, time, s-ip, cs-method, cs-uri-stem, cs-uri-query, s-port, cs-username, c-ip, cs(User-Agent), cs(Referer), sc-status, sc-substatus, sc-win32-status, STRCAT(TO_STRING(time-taken),'ms') INTO %outFile% FROM %logs% WHERE (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) AND sc-status BETWEEN 500 AND 599 ORDER BY cs-method,date,time DESC"
echo Finished %outFile%

set outFile=%resultDir%FailedRequests.gif
%logParser% %commonOptions% "SELECT cs-method AS Method, Count(*) as TotalCount INTO %outFile% FROM %logs% WHERE (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) AND sc-status BETWEEN 500 and 599 GROUP BY Method ORDER BY TotalCount DESC"  -o:CHART -chartType:PieExploded3D -ChartTitle:"Failed Request Methods (%startTime% - %endTime%)" -values:ON
echo Finished %outFile%

set outFile=%resultDir%MethodStats.gif
%logParser% %commonOptions% "SELECT  cs-method, COUNT(*) AS TotalCount, MAX(time-taken) AS MaximumTime, AVG(time-taken) AS AverageTime, MIN(time-taken) AS MinimumTime INTO %outFile% FROM %logs% WHERE (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) GROUP BY cs-method ORDER BY TotalCount DESC" -o:CHART -chartType:ColumnClustered -ChartTitle:"Requests By Method" -groupSize 1920x1080
echo Finished %outFile%

set outFile=%resultDir%HourlyHitsPerSecond.gif
%logParser% %commonOptions% "SELECT TO_STRING(TO_LOCALTIME(TO_TIMESTAMP(date,time)),'yyyy-MM-dd hh') AS LocalTime, DIV(TO_REAL(COUNT(*)), 3600.0) AS HitsPerSecond INTO %outFile% FROM %logs% WHERE (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) GROUP BY LocalTime ORDER BY LocalTime" -o:CHART -chartType:ColumnClustered -chartTitle:"Hourly Hits Per Second" -groupSize 1920x1080
echo Finished %outFile%

set outFile=%resultDir%DailyHits.gif
%logParser% %commonOptions% "SELECT TO_STRING(TO_LOCALTIME(TO_TIMESTAMP(date,time)),'yyyy-MM-dd') AS LocalTime, COUNT(*) AS Hits INTO %outFile% FROM %logs% WHERE (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) GROUP BY LocalTime ORDER BY LocalTime" -o:CHART -chartType:ColumnClustered -chartTitle:"Daily Hits" -groupSize 1920x1080 -values:ON
echo Finished %outFile%

set outFile=%resultDir%StatusCodes.gif
%logParser% %commonOptions% "SELECT TO_STRING(sc-status) AS [HTTP Status Code], COUNT(*) AS Requests INTO %outFile% FROM %logs% WHERE (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) GROUP BY [HTTP Status Code] ORDER BY Requests DESC" -o:CHART -chartType:BarClustered -chartTitle:"Status Code of Requests" -groupSize 1024x768 -values:ON
echo Finished %outFile%

set method=POST
set resource=GwssNotification
set outFile=%resultDir%%resource%%method%%resource%DailyAverages.gif
%logParser% %commonOptions% "SELECT TO_STRING(TO_LOCALTIME(TO_TIMESTAMP(date, time)), 'yyyy-MM-dd') AS [Day], COUNT(*) AS TotalCount, AVG(time-taken) AS AverageTime INTO %outFile% FROM %logs% WHERE (cs-method = '%method%' AND cs-uri-stem LIKE '%%%resource%%%') AND (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) GROUP BY [Day] ORDER BY [Day]" -o:CHART -chartType:ColumnClustered -ChartTitle:"%resource% %method% Request Average Times Taken" -groupSize 1920x1080
echo Finished %outFile%