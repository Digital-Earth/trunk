function EsPut ( $blob ) 
{
    $lastModified = $blob.LastModified.ToString('u').Replace(' ','T')
    $created = $blob.Name.Substring($blob.Name.Length-24,19).Replace('.',':').Replace('-','T') + 'Z'
    $created = $created.Substring(0,4) + '-' + $created.Substring(5,2) + '-' + $created.Substring(8)
    $blob.Name = $blob.Name.Substring(0, $blob.Name.Length - 25)
    $version = [regex]::matches($blob.Name, "\d+\.\d+\.\d+\.\d+")
    if($version.Count -eq 0)
    {
        $version = '0.0.0.0'
        $product = 'WorldView.Studio'
        $fileMethod = $blob.Name.Substring(17)
    }
    else
    {
        $version = $version[0]
        $product = $blob.Name.Substring(0,$version.Index-1)
        $fileMethod = $blob.Name.Substring($version.Index)
        $fileMethod = $fileMethod.Substring($fileMethod.IndexOf('_')+1)
    }
    $file = $fileMethod.Substring(0, $fileMethod.LastIndexOf('.'))
    $method = $fileMethod.Substring($fileMethod.LastIndexOf('.')+1)
    $url = 'http://search.pyxis.worldview.gallery:9200/pyxdiagnostics/crash-dumps/' + $created + '-' + $lastModified

    $putParams = '{"Product":"' + $product + '","Version":"' + $version + '","File":"' + $file + '","Method":"' + $method + '","Length":' + $blob.Length + ',"Created":"' + $created + '","Uploaded":"' + $lastModified + '"}'
    $response = Invoke-WebRequest -Uri $url -Method PUT -Body $putParams
}

 $result = Import-AzurePublishSettingsFile "C:\azure\Idan's BizSpark-6-10-2015-credentials.publishsettings"
 Select-AzureSubscription "Idan's BizSpark"
 $context = New-AzureStorageContext -StorageAccountName "pyxisdiagnosis" -StorageAccountKey "KsHNqWv/MOlkqVe1Y4vDXtgYaLhVRok7Uhf1Ic3LNq1hlXhGWpq/Ap549QDOUZAcfWP59tynqCC8Ey659bWi5Q=="
 $content = Get-AzureStorageBlob -Container crash-dumps -Context $context

 Foreach ($blob in $content)
 {
    $lastUpload = (Get-Date).AddDays(-2)
    if($blob.LastModified -gt $lastUpload)
    {
        EsPut($blob)
    }
 }