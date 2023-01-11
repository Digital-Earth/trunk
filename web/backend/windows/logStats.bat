@echo off
set logParser="C:\Program Files (x86)\Log Parser 2.2\LogParser.exe"
set logs='C:\inetpub\logs\LogFiles\W3SVC1\*.log'
set startYear=2015
set startMonth=04
set startDay=16
set startHour=00
set startMinute=00
set startSecond=00
set endYear=2016
set endMonth=12
set endDay=25
set endHour=23
set endMinute=59
set endSecond=59
set startTime=%startYear%/%startMonth%/%startDay% %startHour%:%startMinute%:%startSecond%
set endTime=%endYear%/%endMonth%/%endDay% %endHour%:%endMinute%:%endSecond%
set resultName=%startYear%_%startMonth%_%startDay%_%startHour%_%startMinute%_%startSecond%-%endYear%_%endMonth%_%endDay%_%endHour%_%endMinute%_%endSecond%
set resultDir=results\%resultName%\
set commonOptions=-i:w3c -q:ON

MKDIR "%resultDir%"

set method=POST
set resource=Register
set outFile=%resultDir%%resource%%method%%resource%Daily.gif
%logParser% %commonOptions% "SELECT TO_STRING(TO_LOCALTIME(TO_TIMESTAMP(date, time)), 'yyyy-MM-dd') AS [Day], COUNT(*) AS TotalCount INTO %outFile% FROM %logs% WHERE (cs-method = '%method%' AND cs-uri-stem LIKE '%%%resource%%%') AND (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) GROUP BY [Day] ORDER BY [Day]" -o:CHART -chartType:ColumnClustered -ChartTitle:"%resource% %method% Requests" -groupSize 1920x1080
echo Finished %outFile%

set method=POST
set resource=Register
set outFile=%resultDir%%resource%%method%%resource%.txt
%logParser% %commonOptions% "SELECT * INTO %outFile% FROM %logs% WHERE (cs-method = '%method%' AND cs-uri-stem LIKE '%%%resource%%%') AND (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss')))"
echo Finished %outFile%

set outFile=%resultDir%cache.gif
%logParser% %commonOptions% "SELECT sc-status AS Status, COUNT(*) AS Count INTO %outFile% FROM %logs% WHERE (sc-status=200 OR sc-status=304) AND (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) GROUP BY Status ORDER BY Status" -o:CHART -chartType:PieExploded3D -ChartTitle:"Cache" -values:ON
echo Finished %outFile%

set outFile=%resultDir%statusCodes.gif
%logParser% %commonOptions% "SELECT STRCAT(TO_STRING(sc-status), STRCAT('.', TO_STRING(sc-substatus))) AS Status, Count(*) as Hits INTO %outFile% FROM %logs% WHERE (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) GROUP BY Status ORDER BY Hits DESC"  -o:CHART -chartType:ColumnClustered -ChartTitle:"Status.SubStatus Codes" -groupSize 1920x1080
echo Finished %outFile%

set outFile=%resultDir%FailedRequests.txt
%logParser% %commonOptions% "SELECT date, time, s-ip, cs-method, cs-uri-stem, cs-uri-query, s-port, cs-username, c-ip, cs(User-Agent), cs(Referer), sc-status, sc-substatus, sc-win32-status, time-taken INTO %outFile% FROM %logs% WHERE (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) AND sc-status BETWEEN 500 AND 599 ORDER BY cs-method,date,time DESC"
echo Finished %outFile%

set outFile=%resultDir%Browsers.txt
%logParser% %commonOptions% "SELECT to_int(mul(100.0,PropCount(*))) as Percent, count(*) as TotalHits, cs(User-Agent) as Browser INTO %outFile% FROM %logs% WHERE (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) AND sc-status BETWEEN 500 and 599 GROUP BY Browser ORDER BY TotalHits DESC" 
echo Finished %outFile%

set outFile=%resultDir%FailedRequests.gif
%logParser% %commonOptions% "SELECT cs-method AS Method, Count(*) as TotalCount INTO %outFile% FROM %logs% WHERE (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) AND sc-status BETWEEN 500 and 599 GROUP BY Method ORDER BY TotalCount DESC"  -o:CHART -chartType:PieExploded3D -ChartTitle:"Failed Request Methods (%startTime% - %endTime%)" -values:ON
echo Finished %outFile%

set outFile=%resultDir%methodStats.gif
%logParser% %commonOptions% "SELECT  cs-method, COUNT(*) AS TotalCount, MAX(time-taken) AS MaximumTime, AVG(time-taken) AS AverageTime, MIN(time-taken) AS MinimumTime INTO %outFile% FROM %logs% WHERE (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) GROUP BY cs-method ORDER BY TotalCount DESC" -o:CHART -chartType:ColumnClustered -ChartTitle:"Requests By Method" -groupSize 1920x1080
echo Finished %outFile%

set outFile=%resultDir%timeHist.txt
%logParser% %commonOptions% "SELECT  cs-method, cs-uri-stem, COUNT(*) AS TotalCount, MAX(time-taken) AS MaximumTime, AVG(time-taken) AS AverageTime, MIN(time-taken) AS MinimumTime INTO %outFile% FROM %logs% WHERE (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss')))  GROUP BY cs-uri-stem, cs-method ORDER BY MaximumTime DESC"
echo Finished %outFile%

set outFile=%resultDir%queryTimeHist.txt
%logParser% %commonOptions% "SELECT  cs-method, cs-uri-stem, cs-uri-query, COUNT(*) AS TotalCount, MAX(time-taken) AS MaximumTime, AVG(time-taken) AS AverageTime, MIN(time-taken) AS MinimumTime INTO %outFile% FROM %logs% WHERE (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss')))  GROUP BY cs-uri-stem, cs-uri-query, cs-method ORDER BY MaximumTime DESC"
echo Finished %outFile%

set method=GET
set outFile=%resultDir%timeHist%method%.gif
%logParser% %commonOptions% "SELECT cs-uri-stem, COUNT(*) AS TotalCount, MAX(time-taken) AS MaximumTime, AVG(time-taken) AS AverageTime, MIN(time-taken) AS MinimumTime INTO %outFile% FROM %logs% WHERE (cs-method = '%method%') AND (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss')))  GROUP BY cs-uri-stem ORDER BY MaximumTime DESC" -o:CHART -chartType:ColumnClustered -ChartTitle:"%method% Request Times Taken" -groupSize 1920x1080
echo Finished %outFile%

set method=POST
set outFile=%resultDir%timeHist%method%.gif
%logParser% %commonOptions% "SELECT cs-uri-stem, COUNT(*) AS TotalCount, MAX(time-taken) AS MaximumTime, AVG(time-taken) AS AverageTime, MIN(time-taken) AS MinimumTime INTO %outFile% FROM %logs% WHERE (cs-method = '%method%') AND (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss')))  GROUP BY cs-uri-stem ORDER BY MaximumTime DESC" -o:CHART -chartType:ColumnClustered -ChartTitle:"%method% Request Times Taken" -groupSize 1920x1080
echo Finished %outFile%

set outFile=%resultDir%timeHistAll.gif
%logParser% %commonOptions% "SELECT EXTRACT_PATH(cs-uri-stem) AS [Path Requested], COUNT(*) AS TotalCount, MAX(time-taken) AS MaximumTime, AVG(time-taken) AS AverageTime, MIN(time-taken) AS MinimumTime INTO %outFile% FROM %logs% WHERE (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss')))  GROUP BY [Path Requested] ORDER BY MaximumTime DESC" -o:CHART -chartType:BarClustered -ChartTitle:"Request Times Taken" -groupSize 1920x1080
echo Finished %outFile%

set outFile=%resultDir%MinutelyHitsPerSecond.gif
%logParser% %commonOptions% "SELECT TO_STRING(TO_LOCALTIME(TO_TIMESTAMP(date,time)),'yyyy-MM-dd hh:mm') AS LocalTime, DIV(TO_REAL(COUNT(*)), 60.0) AS HitsPerSecond INTO %outFile% FROM %logs% WHERE (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss')))  GROUP BY LocalTime ORDER BY LocalTime" -o:CHART -chartType:ColumnClustered -chartTitle:"Minutely Hits Per Second" -groupSize 1920x1080
echo Finished %outFile%

set outFile=%resultDir%HourlyHitsPerSecond.gif
%logParser% %commonOptions% "SELECT TO_STRING(TO_LOCALTIME(TO_TIMESTAMP(date,time)),'yyyy-MM-dd hh') AS LocalTime, DIV(TO_REAL(COUNT(*)), 3600.0) AS HitsPerSecond INTO %outFile% FROM %logs% WHERE (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) GROUP BY LocalTime ORDER BY LocalTime" -o:CHART -chartType:ColumnClustered -chartTitle:"Hourly Hits Per Second" -groupSize 1920x1080
echo Finished %outFile%

set outFile=%resultDir%DailyHits.gif
%logParser% %commonOptions% "SELECT TO_STRING(TO_LOCALTIME(TO_TIMESTAMP(date,time)),'yyyy-MM-dd') AS LocalTime, COUNT(*) AS Hits INTO %outFile% FROM %logs% WHERE (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) GROUP BY LocalTime ORDER BY LocalTime" -o:CHART -chartType:ColumnClustered -chartTitle:"Daily Hits" -groupSize 1920x1080 -values:ON
echo Finished %outFile%

set outFile=%resultDir%statusCodes.gif
%logParser% %commonOptions% "SELECT TO_STRING(sc-status) AS [HTTP Status Code], COUNT(*) AS Requests INTO %outFile% FROM %logs% WHERE (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) GROUP BY [HTTP Status Code] ORDER BY Requests DESC" -o:CHART -chartType:BarClustered -chartTitle:"Status Code of Requests" -groupSize 1024x768 -values:ON
echo Finished %outFile%

set method=POST
set resource=GwssNotification
set outFile=%resultDir%%resource%%method%%resource%HourlyAverages.gif
%logParser% %commonOptions% "SELECT TO_STRING(TO_LOCALTIME(TO_TIMESTAMP(date, time)), 'yyyy-MM-dd hh') AS [Hour], COUNT(*) AS TotalCount, AVG(time-taken) AS AverageTime INTO %outFile% FROM %logs% WHERE (cs-method = '%method%' AND cs-uri-stem LIKE '%%%resource%%%') AND (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) GROUP BY [Hour] ORDER BY [Hour]" -o:CHART -chartType:ColumnClustered -ChartTitle:"%resource% %method% Average Hourly Request Times Taken" -groupSize 1920x1080
echo Finished %outFile%

set method=POST
set resource=GwssNotification
set outFile=%resultDir%%resource%%method%%resource%DailyTimes.gif
%logParser% %commonOptions% "SELECT TO_STRING(TO_LOCALTIME(TO_TIMESTAMP(date, time)), 'yyyy-MM-dd') AS [Day], COUNT(*) AS TotalCount, MAX(time-taken) AS MaximumTime, AVG(time-taken) AS AverageTime, MIN(time-taken) AS MinimumTime INTO %outFile% FROM %logs% WHERE (cs-method = '%method%' AND cs-uri-stem LIKE '%%%resource%%%') AND (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) GROUP BY [Day] ORDER BY [Day]" -o:CHART -chartType:ColumnClustered -ChartTitle:"%resource% %method% Request Times Taken" -groupSize 1920x1080
echo Finished %outFile%

set method=POST
set resource=GwssNotification
set outFile=%resultDir%%resource%%method%%resource%DailyAverages.gif
%logParser% %commonOptions% "SELECT TO_STRING(TO_LOCALTIME(TO_TIMESTAMP(date, time)), 'yyyy-MM-dd') AS [Day], COUNT(*) AS TotalCount, AVG(time-taken) AS AverageTime INTO %outFile% FROM %logs% WHERE (cs-method = '%method%' AND cs-uri-stem LIKE '%%%resource%%%') AND (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss'))) GROUP BY [Day] ORDER BY [Day]" -o:CHART -chartType:ColumnClustered -ChartTitle:"%resource% %method% Request Average Times Taken" -groupSize 1920x1080
echo Finished %outFile%

set outFile=%resultDir%timeStats.txt
%logParser% %commonOptions% "SELECT cs-uri-stem, time-taken INTO %outFile% FROM %logs% WHERE (TO_TIMESTAMP(date,time) between TO_UTCTIME(TIMESTAMP('%startTime%', 'yyyy/MM/dd hh:mm:ss')) AND TO_UTCTIME(TIMESTAMP('%endTime%', 'yyyy/MM/dd hh:mm:ss')))  ORDER BY time-taken DESC"
echo Finished %outFile%