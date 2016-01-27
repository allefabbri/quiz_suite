$n = $args.count
If ( $args.count -lt 3 ){
    "Usage : {0} input output size_of_blank" -f $MyInvocation.MyCommand.Name #printf on PS
    Exit
}

Write-Host "Input      :"$args[0]
Write-Host "Output     :"$args[1]
Write-Host "Blank size :"$args[2]
$blank = "-"*$args[2]

$content = Get-Content $args[0]
$out = ""
foreach ($line in $content)
{
    $tokens = $line.Split(";")
    $out += $tokens[0]+"`t`t"+$blank+"`t`t"+$tokens[2]+"`t`t"+$tokens[3]+"`n"
}

$out | Out-File $args[1]
