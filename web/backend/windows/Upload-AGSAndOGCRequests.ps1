Function Upload-AGSAndOGCRequests {
    Param (
        [Parameter(Mandatory=$true)]
        [String]$FilePath
        )

    $body = "";
    foreach ($line in [System.IO.File]::ReadLines($FilePath)) {
        $values = $line.Split(" ")| where {$_}
        $body += ("{""Count"":" + $values[0] + ",""Uri"":""" + $values[2] + """},").Replace("""""","""") 
    }
    if($body.Length -gt 0) {
        $body = $body.Substring(0, $body.Length-1)
        $url = 'https://crawler.pyxis.worldview.gallery/api/Requests'
        $body = '[' + $body + ']'
        $response = Invoke-WebRequest -Uri $url -Method POST -Body $body -Headers  @{"Authorization"="Basic UHl4aXM6SW5ub3ZhdGlvbjE=";"Content-Type"="application/json"}
    }

} #End Function Upload-AGSAndOGCRequests