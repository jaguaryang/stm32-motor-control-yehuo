# 获取当前目录下所有符合条件的文件
$files = Get-ChildItem -Path . -Recurse -Include *.txt,*.c,*.h | 
    Where-Object { 
        $_.FullName -notmatch '\\Drivers\\' -and 
        $_.FullName -notmatch '\\Libraries\\' -and 
        $_.FullName -notmatch '\\Project\\' 
    }

foreach ($file in $files) {
    try {
        # 读取文件内容
        $content = Get-Content -Path $file.FullName -Raw -ErrorAction Stop
        
        # 检测文件编码
        $encoding = [System.Text.Encoding]::Default
        $bytes = [System.Text.Encoding]::Default.GetBytes($content)
        
        # 检查是否已经是UTF-8
        $isUtf8 = $false
        try {
            $utf8Content = [System.Text.Encoding]::UTF8.GetString($bytes)
            $testBytes = [System.Text.Encoding]::UTF8.GetBytes($utf8Content)
            $isUtf8 = @(Compare-Object $bytes $testBytes -SyncWindow 0).Length -eq 0
        }
        catch {
            $isUtf8 = $false
        }
        
        # 如果不是UTF-8，则转换
        if (-not $isUtf8) {
            Write-Host "Converting $($file.FullName) to UTF-8"
            
            # 将内容转换为UTF-8并保存
            $utf8NoBOM = New-Object System.Text.UTF8Encoding $false
            [System.IO.File]::WriteAllLines($file.FullName, $content, $utf8NoBOM)
            
            Write-Host "Converted successfully" -ForegroundColor Green
        }
        else {
            Write-Host "$($file.FullName) is already UTF-8" -ForegroundColor Gray
        }
    }
    catch {
        Write-Host "Error processing $($file.FullName): $($_.Exception.Message)" -ForegroundColor Red
    }
}

Write-Host "`nConversion process completed" -ForegroundColor Green