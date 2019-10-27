using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace Launcher.Pages
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class NewGame : Page
    {
        public NewGame()
        {
            this.InitializeComponent();
            BotEnabled.IsOn = true;
            RadioButton_Easy.IsChecked = true;
        }

        private void BotEnabled_Toggled(object sender, RoutedEventArgs e)
        {
            BOTConfigPanel.Visibility = BotEnabled.IsOn ? Visibility.Visible: Visibility.Collapsed;
            
        }

        private void GameModeComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            
        }

        private void MapComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {

        }

        [DllImport("csmoe_winrt.dll")]
        public static extern int WinRT_InitWithXaml(SwapChainBackgroundPanel hwnd);
        //public static extern int Host_Main(int szArgc, string[] szArgv, string szGameDir, int chg, IntPtr callback);
        //public delegate int SDL_main_func(int szArgc, string[] szArgv);
        //public static extern int SDL_WinRTRunApp(SDL_main_func mainFunction, object xamlBackgroundPanel);

        private void StartGameBtn_Click(object sender, RoutedEventArgs e)
        {
            //XashLauncher.launchGame();
            WinRT_InitWithXaml(BgPanel);
        }

        private void CancelSelectionBtn_Click(object sender, RoutedEventArgs e)
        {
            if (this.Frame.CanGoBack)
                this.Frame.GoBack();
        }
    }
}
