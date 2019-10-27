
using System.Runtime.InteropServices;
using Windows.UI.Xaml.Controls;
using Windows.Storage;
using System;
using System.Collections;

namespace Launcher
{
    public class XashLauncher
    {
        private static StorageFolder m_GameDir;
        public static StorageFolder GameDir
        {
            get
            {
                try
                {
                    return m_GameDir != null ? m_GameDir : StorageFolder.GetFolderFromPathAsync(ApplicationData.Current.LocalSettings.Values["GameDir"].ToString()).GetResults();
                }
                catch (Exception e)
                {
                    return null;
                }
            }
            set
            {
                ApplicationData.Current.LocalSettings.Values["GameDir"] = (m_GameDir = value).Path;
                ApplicationData.Current.SignalDataChanged();
            }
        }
        
        public async static void launchGame()
        {
            //StorageFile exe = await GameDir.GetFileAsync("csmoe_win32.exe");
            await Windows.ApplicationModel.FullTrustProcessLauncher.LaunchFullTrustProcessForCurrentAppAsync();
            try
            {
                await Windows.System.ProcessLauncher.RunToCompletionAsync("csmoe_win32.exe -gamedir \"" + GameDir.Path + "\" -game csbtem", "");
                await Windows.System.ProcessLauncher.RunToCompletionAsync("csmoe_win32.exe -rootdir D:\\CSBTE-Mobile", "-rootdir D:\\CSBTE-Mobile");
                
            } catch(Exception e)
            {

            }
            
            
            // ...
        }
    }
}
