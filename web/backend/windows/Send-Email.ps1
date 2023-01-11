Function Send-EMail {
    Param (
        [Parameter(Mandatory=$true)]
        [String]$EmailTo,
        [Parameter(Mandatory=$true)]
        [String]$Subject,
        [Parameter(Mandatory=$true)]
        [String]$Body,
        [Parameter(Mandatory=$false)]
        [String]$EmailFrom="wvbackendapi0001@gmail.com",  #This gives a default value to the $EmailFrom command
        [Parameter(mandatory=$false)]
        [String]$Attachments,
        [Parameter(mandatory=$true)]
        [String]$Password,
        [Parameter(mandatory=$false)]
        [Boolean]$HTMLBody=$false
    )

        $SMTPServer = "smtp.gmail.com" 
        $SMTPMessage = New-Object System.Net.Mail.MailMessage($EmailFrom,$EmailTo,$Subject,$Body)
        $SMTPMessage.IsBodyHtml = $HTMLBody
        if ($attachments) {
            foreach($attachment in $attachments.Split(",")) {
                #$SMTPattachment = New-Object System.Net.Mail.Attachment($attachment)
                $SMTPMessage.Attachments.Add($attachment)
            }
        }
        $SMTPClient = New-Object Net.Mail.SmtpClient($SmtpServer, 587) 
        $SMTPClient.EnableSsl = $true 
        $SMTPClient.Credentials = New-Object System.Net.NetworkCredential($EmailFrom.Split("@")[0], $Password); 
        $SMTPClient.Send($SMTPMessage)
        Remove-Variable -Name SMTPClient
        Remove-Variable -Name Password

} #End Function Send-EMail