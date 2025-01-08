# 获取当前目录下所有符合条件的文件
$files = Get-ChildItem -Path . -Recurse -Include *.txt,*.c,*.h | 
    Where-Object { 
        $_.FullName -notmatch '\\Drivers\\' -and 
        $_.FullName -notmatch '\\Libraries\\' -and 
        $_.FullName -notmatch '\\Project\\' 
    }

# 创建编码对象
$gb2312 = [System.Text.Encoding]::GetEncoding('GB2312')

# 检测文件是否是 UTF-8 编码的函数
function Test-FileIsUTF8 {
    param (
        [string]$FilePath
    )
    
    try {
        $bytes = [System.IO.File]::ReadAllBytes($FilePath)
        
        # 检查是否有 UTF-8 BOM
        if ($bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF) {
            return $true
        }
        
        # 检查是否是无 BOM 的 UTF-8
        $utf8Valid = $true
        for ($i = 0; $i -lt $bytes.Length; $i++) {
            # 检查多字节序列
            if ($bytes[$i] -gt 0x7F) {
                # 获取前导1的数量，确定字节序列长度
                $byteCount = 0
                $temp = $bytes[$i]
                while ($temp -band 0x80) {
                    $byteCount++
                    $temp = $temp -shl 1
                }
                
                # 检查后续字节
                if ($i + $byteCount -gt $bytes.Length) {
                    $utf8Valid = $false
                    break
                }
                
                for ($j = 1; $j -lt $byteCount; $j++) {
                    if (($bytes[$i + $j] -band 0xC0) -ne 0x80) {
                        $utf8Valid = $false
                        break
                    }
                }
                
                if (-not $utf8Valid) {
                    break
                }
                
                $i += $byteCount - 1
            }
        }
        
        return $utf8Valid
    }
    catch {
        return $false
    }
}

foreach ($file in $files) {
    try {
        Write-Host "Processing $($file.FullName)"
        
        # 首先检查文件是否已经是 UTF-8
        if (Test-FileIsUTF8 -FilePath $file.FullName) {
            Write-Host "File is already UTF-8, skipping..." -ForegroundColor Gray
            continue
        }
        
        # 使用 GB2312 编码读取文件内容
        $bytes = [System.IO.File]::ReadAllBytes($file.FullName)
        $content = $gb2312.GetString($bytes)
        
        # 使用 UTF-8 无 BOM 编码保存
        $utf8NoBOM = New-Object System.Text.UTF8Encoding $false
        [System.IO.File]::WriteAllText($file.FullName, $content, $utf8NoBOM)
        
        Write-Host "Converted successfully" -ForegroundColor Green
    }
    catch {
        Write-Host "Error processing $($file.FullName): $($_.Exception.Message)" -ForegroundColor Red
    }
}

Write-Host "`nConversion process completed" -ForegroundColor Green