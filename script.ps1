

# Check if the script is running as administrator
if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    # Relaunch the script with elevated permissions
    Start-Process powershell -WindowStyle Hidden -ArgumentList "-File `"$PSCommandPath`"" -Verb RunAs
} else {
    # Your script code here
    Write-Host "Running with administrator privileges!"
    # Continue with the rest of your script code
    Add-Type -AssemblyName System.Windows.Forms
    Add-Type -AssemblyName System.Drawing
    Add-Type -AssemblyName System.Management

    # Define function to get network adapters
    function Get-NetworkAdapters {
        $adapters = Get-WmiObject Win32_NetworkAdapter | Where-Object { $_.NetConnectionStatus -eq 2 }
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

    # Define the form
    $form = New-Object System.Windows.Forms.Form
    $form.Text = "System Tray App"
    $form.Size = New-Object System.Drawing.Size(0, 0)

    # Define the notify icon
    $notifyIcon = New-Object System.Windows.Forms.NotifyIcon
    $iconPath = "C:\Users\shoccho\Downloads\computer.ico"  # Replace with the path to your icon file
    $notifyIcon.Icon = [System.Drawing.Icon]::ExtractAssociatedIcon($iconPath)
    $notifyIcon.Visible = $true

    # Define the context menu
    $contextMenu = New-Object System.Windows.Forms.ContextMenuStrip

    # Add network adapter options to the context menu
    $networkAdapters = Get-NetworkAdapters
    foreach ($adapterName in $networkAdapters) {
        $menuItem = New-Object System.Windows.Forms.ToolStripMenuItem
        $menuItem.Text = $adapterName
        $menuItem.Add_Click({ Disable-NetworkAdapter $adapterName })
        $contextMenu.Items.Add($menuItem)
    }

    # Add exit option to the context menu
    $exitOption = New-Object System.Windows.Forms.ToolStripMenuItem
    $exitOption.Text = "Exit"
    $exitOption.Add_Click({ $form.Close() })
    $contextMenu.Items.Add($exitOption)

    # Set the context menu for the notify icon
    $notifyIcon.ContextMenuStrip = $contextMenu

    # Show the form
    $form.Add_Shown({ $form.WindowState = "Minimized" })
    
    $objForm.Visible = $false
    $objForm.WindowState = "minimized"
    $objForm.ShowInTaskbar = $false
    $objForm.add_Closing({ $objForm.ShowInTaskBar = $false })

    $form.ShowDialog() | Out-Null
}