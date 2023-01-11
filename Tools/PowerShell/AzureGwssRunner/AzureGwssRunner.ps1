#----------------------------------------------------
# Magic Values:
#----------------------------------------------------

$gwssExeFile = "geostreamservice.exe"
$settings = ".\AzureGwssSettings.xml"
$getIPUrl = "http://ifconfig.me/ip"

#----------------------------------------------------
# Setup config XML
#----------------------------------------------------
if (!(Test-Path $settings))
{
   #----------------------------------------------------
   # generate config values
   #----------------------------------------------------
   echo "creating Server settings..."

   # create new GUID
   $g = [guid]::NewGuid()
   echo ("Server ID: "+$g)

   # get public IP.
   $wc=new-object system.net.webclient
   $ip = ($wc.DownloadString($getIPUrl).Trim() + ":44017")
   echo ("Server public IP: "+$ip)

   # find the latest version of GWSS to run - we should use launcher to download it for u.
   $allexe = Get-ChildItem -Path . -Filter $gwssExeFile -Recurse | sort LastWriteTime -Descending
   $exe = $allexe[0].FullName
   
   $xml = [xml]("<AzureGwssSettings><ServerId>"+$g+"</ServerId><ServerIP>"+$ip+"</ServerIP><ServerExe>"+$exe+"</ServerExe></AzureGwssSettings>")
   $xml.save($settings)

   echo "settings were saved"
}
else
{
   #----------------------------------------------------
   # load config file and get the right Values from it.
   #----------------------------------------------------
   $xml = [xml](Get-Content $settings)
   $g = $xml.AzureGwssSettings.ServerId
   $ip = $xml.AzureGwssSettings.ServerIP
   $exe = $xml.AzureGwssSettings.ServerExe
}

#----------------------------------------------------
# dispaly settings
#----------------------------------------------------
echo $g $ip $exe


#----------------------------------------------------
# add firewall setting 
# http://blogs.msdn.com/b/tomholl/archive/2010/11/08/adding-a-windows-firewall-rule-using-powershell.aspx
#----------------------------------------------------
function Add-FirewallRule {
   param( 
      $name,
      $tcpPorts,
      $appName,
      $serviceName = $null
   )
    $fw = New-Object -ComObject hnetcfg.fwpolicy2 
 
    #----------------------------------------------------
    # add firewall ruule only once...
    #----------------------------------------------------
    if (!($fw.Rules | where {$_.ApplicationName -eq $appName})) { 
        $rule = New-Object -ComObject HNetCfg.FWRule
        $rule.Name = $name
        $rule.ApplicationName = $appName
        if ($serviceName -ne $null) { $rule.serviceName = $serviceName }
        $rule.Protocol = 6 #NET_FW_IP_PROTOCOL_TCP
        $rule.LocalPorts = $tcpPorts
        $rule.Enabled = $true
        $rule.Grouping = "@firewallapi.dll,-23255"
        $rule.Profiles = 7 # all
        $rule.Action = 1 # NET_FW_ACTION_ALLOW
        $rule.EdgeTraversal = $false
 
        $fw.Rules.Add($rule) 
        echo ("adding new Firewall Rule for " + $appName)
    } else { 
        echo "Firewall Rule already exsits"
    }
}
Add-FirewallRule $exe "*" $exe $null

#----------------------------------------------------
# set up geostreamservice.exe.config
#----------------------------------------------------
$configPath = ($exe+".config")
$config = [xml](Get-Content $configPath)
$ipSetting = $config.SelectSingleNode("//setting[@name = 'ServerExternalIPAddress']")
$gSetting = $config.SelectSingleNode("//setting[@name = 'ServerId']")

$ipSetting.value = $ip
$gSetting.value = $g.ToString()

$config.Save($configPath)

#----------------------------------------------------
# run Gwss forever
#----------------------------------------------------
while(1) { Start-Process $exe -Wait }