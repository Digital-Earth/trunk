echo on
$rootDir = "C:\LogStats"
. C:\LogStats\Send-Email.ps1
. C:\LogStats\Read-NLines.ps1
. C:\LogStats\Upload-AGSAndOGCRequests.ps1

$periodDays = 7
#separate multiple addresses by commas
$emailTo = "lrakai@pyxisinnovation.com,shatzi@pyxisinnovation.com,nhamekasi@pyxisinnovation.com,rtaylor@pyxisinnovation.com,yklymko@pyxisinnovation.com,myoung@pyxisinnovation.com"
#$emailTo = "lrakai@pyxisinnovation.com"
$server = "wvbackendapi0001"
$emailFrom = $server + "@gmail.com"

$end = (Get-Date -Format u) -replace "Z$"
$start = (Get-Date ((Get-Date).AddDays(-1*$periodDays)) -Format u) -replace "Z$"
$resultDir = ("\results\weekly_$start\") -replace "-","_" -replace " ","_" -replace ":","_"
$resultDir = $rootDir + $resultDir
$arguments = $start, $end, "$resultDir"
$startTime = Get-Date
cd $rootDir
.\weeklyStats.bat $arguments 
$elapsed = (New-TimeSpan -Start $startTime -End (Get-Date)).TotalSeconds

$N = 10
$body = "<h2>PYXIS Analytics for $server</h2>Generating this report took {0}s.`n`n" -f $elapsed
$body += "Active Users: " + (Get-Content ($resultDir + "UserActivity.txt") | Measure-Object -Line).Lines.ToString() + "`n"
$totalRequests = (Get-Content ($resultDir + "TotalRequests.txt"))
$body += "Total Requests: " + $totalRequests + "`n"
$failedRequests = (Get-Content ($resultDir + "FailedRequests.txt") | Measure-Object -Line).Lines
$totalRequests = $totalRequests.Split(" ")[0]
$body += "Failed Requests: " + $failedRequests.ToString() + " (" + ($failedRequests / [int]$totalRequests * 100).ToString() + "%)`n`n"

$body += "Top $N Requests`n===============`n"
$body += (Read-NLines -FilePath ($resultDir + "TopRequests.txt") -N $N) + "`n`n"

$body += "Top $N Searches`n============`n"
$body += (Read-NLines -FilePath ($resultDir + "TopSearches.txt") -N $N) + "`n`n"

$body += "Top $N Users`n============`n"
$body += (Read-NLines -FilePath ($resultDir + "UserActivity.txt") -N $N) + "`n`n"

$body += "Top $N AGS and OGC GetCapabilities`n============`n"
$body += (Read-NLines -FilePath ($resultDir + "TopAGSAndOGCSearches.txt") -N $N) + "`n`n"

$body += "Failed Requests`n============`n"
$body += (Read-NLines -FilePath ($resultDir + "FailedRequests.txt") -N 50) + "`n`n"

$html = "<!DOCTYPE html PUBLIC ""-//W3C//DTD XHTML 1.0 Strict//EN""  ""http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"">
<html xmlns=""http://www.w3.org/1999/xhtml"">
<head>
<title>$server Week Ending $($end.Substring(0,10))</title>
</head><body>"
$html += $body.Replace("`n","<br />") + "<h5>You are receiving this because you are a member of [LS-Monitoring]. <a href=""mailto:lrakai@pyxisinnovation.com?subject=[LS-Monitoring]Unsubscribe"">Unsubscribe</a> if you would like to stop receiving these emails.</h5></body></html>"

$attachments = ($resultDir + "UsersPerDay.gif") + "," `
    + ($resultDir + "DailyHits.gif") + "," `
    + ($resultDir + "GwssNotificationPOSTGwssNotificationDailyAverages.gif") + "," `
    + ($resultDir + "HourlyHitsPerSecond.gif") + "," `
    + ($resultDir + "MethodStats.gif") + "," `
    + ($resultDir + "StatusCodes.gif") + "," `
    + ($resultDir + "FailedRequests.gif")
Send-EMail -EmailTo $emailTo -Body $html -Subject "[LS-Monitoring] $server Week Ending $($end.Substring(0,10))" -EmailFrom $emailFrom -Password "Innovation1" -HTMLBody $true -Attachments $attachments

Upload-AGSAndOGCRequests -FilePath ($resultDir + "TopAGSAndOGCSearches.txt")