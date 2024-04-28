if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {

    Start-Process powershell -WindowStyle Hidden -ArgumentList "-File `"$PSCommandPath`"" -Verb RunAs
} else {
    Add-Type -AssemblyName System.Windows.Forms
    Add-Type -AssemblyName System.Drawing
    Add-Type -AssemblyName System.Management

    function Get-NetworkAdapters {
        $adapters = Get-WmiObject Win32_NetworkAdapter
        $adapterNames = @()
        foreach ($adapter in $adapters) {
            $adapterNames += $adapter.NetConnectionID
        }
        return $adapterNames
    }

    function Disable-NetworkAdapter {
        param($adapterName)
        try {
            $adapter = Get-WmiObject Win32_NetworkAdapter | Where-Object { $_.NetConnectionID -eq $adapterName }
            if ($adapter) {
                $enabled = $adapterIndex = Get-NetAdapter | % { Process { If (( $_.Status -eq "up" ) -and ($_.Name -eq $adapterName ) ){ $true } }};

                if($enabled){
                    Disable-NetAdapter $adapterName -Confirm:$False
                    Write-Host "Network adapter '$adapterName' disabled."
                   }else{
                     Enable-NetAdapter $adapterName -Confirm:$False
                    Write-Host "Network adapter '$adapterName' enabled."
                   }
                } else {
                Write-Host "Network adapter '$adapterName' not found."
            }
        } catch {
            Write-Host "Error disabling network adapter '$adapterName': $_"
        }
    }

    $iconPath = (Get-Process -Id $PID).Path
    $tooltip = "Network Manager: Click on a network adapter name to turn it on or off."

    $notifyIcon = [System.Windows.Forms.NotifyIcon]::new()
    $notifyIcon.Icon = [System.Drawing.Icon]::ExtractAssociatedIcon($iconPath)
    $notifyIcon.Text = $tooltip
   
    $done = $false

    $contextMenu = [System.Windows.Forms.ContextMenuStrip]::new()

    $networkAdapters = Get-NetworkAdapters
    foreach ($adapterName in $networkAdapters) {
        $menuItem = New-Object System.Windows.Forms.ToolStripMenuItem
        $menuItem.Text = $adapterName
        $menuItem.Add_Click({ Disable-NetworkAdapter $adapterName })
        $contextMenu.Items.Add($menuItem)
    }
    $menuItemExit = [System.Windows.Forms.ToolStripMenuItem]::new()
    $menuItemExit.Text = "Quit."
    $null = $contextMenu.Items.Add($menuItemExit)
    $notifyIcon.ContextMenuStrip = $contextMenu
    $menuItemExit.add_Click({ $script:done = $true })

    $notifyIcon.Visible = $true
    try {
        while (-not $done) {
            [System.Windows.Forms.Application]::DoEvents()
            Start-Sleep -MilliSeconds 100
        }
    }
    finally {
        $notifyIcon.Dispose()
        Write-Verbose -Verbose 'Exiting.'
    }
}