if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {

    Start-Process powershell -WindowStyle Hidden -ArgumentList "-File `"$PSCommandPath`"" -Verb RunAs
} else {
    Add-Type -AssemblyName System.Windows.Forms
    Add-Type -AssemblyName System.Drawing
    Add-Type -AssemblyName System.Management

    function Toggle-NetworkAdapter {
        param($adapterName)
        try {
            $adapter = Get-WmiObject Win32_NetworkAdapter | Where-Object { $_.NetConnectionID -eq $adapterName }
            if ($adapter) {
                $enabled = Get-NetAdapter | % { Process { If (( $_.Status -eq "up" ) -and ($_.Name -eq $adapterName ) ){ $true } }};

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
        Refresh-ContextMenu
    }

    $iconPath = (Get-Process -Id $PID).Path
    $tooltip = "Network Manager: Click on a network adapter name to turn it on or off."

    $notifyIcon = [System.Windows.Forms.NotifyIcon]::new()
    $notifyIcon.Icon = [System.Drawing.Icon]::ExtractAssociatedIcon($iconPath)
    $notifyIcon.Text = $tooltip
   
    $done = $false

    $contextMenu = [System.Windows.Forms.ContextMenuStrip]::new()

    function Refresh-ContextMenu {
        $contextMenu.Items.Clear()

        $networkAdapters = Get-WmiObject Win32_NetworkAdapter | Where-Object { $_.NetConnectionStatus -in 0, 2, 5 -and -not [string]::IsNullOrEmpty($_.NetConnectionID) }
        foreach ($adapter in $networkAdapters) {
            $localAdapterName = $adapter.NetConnectionID

            $menuItem = New-Object System.Windows.Forms.ToolStripMenuItem
            $menuItem.Text = $localAdapterName
            $menuItem.Checked = ($adapter.NetConnectionStatus -eq 2)
            $menuItem.Tag = $localAdapterName
            $menuItem.Add_Click({
                param($sender, $eventArgs)
                $adapterName = $sender.Tag
                Toggle-NetworkAdapter $adapterName
            })
            $contextMenu.Items.Add($menuItem)
        }

        $menuItemExit = [System.Windows.Forms.ToolStripMenuItem]::new()
        $menuItemExit.Text = "Quit"
        $null = $contextMenu.Items.Add($menuItemExit)
        $menuItemExit.add_Click({ $script:done = $true })
    }
    $notifyIcon.ContextMenuStrip = $contextMenu
    Refresh-ContextMenu
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