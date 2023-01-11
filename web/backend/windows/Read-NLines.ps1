Function Read-NLines {
    Param (
        [Parameter(Mandatory=$true)]
        [String]$FilePath,
        [Parameter(Mandatory=$true)]
        [Int]$N
        )

$reader = [System.IO.File]::OpenText($FilePath)
$text = "";
for($i = 0; $i -lt $N; $i++){ 
    if(-Not ($reader.EndOfStream)) {
        $text += $reader.ReadLine() + "`n"
    }
}
$reader.Close()

return $text

} #End Function ReadNLines