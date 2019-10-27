using Launcher.Pages;
using Launcher.Presentation;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Media.Animation;
using Windows.UI.Xaml.Navigation;

namespace Launcher
{
    public sealed partial class Shell : UserControl
    {
        public Shell()
        {
            this.InitializeComponent();

            var vm = new ShellViewModel();
            vm.MenuItems.Add(new MenuItem { Icon = ((char)Symbol.Home).ToString(), Title = "Welcome", PageType = typeof(WelcomePage) });
            vm.MenuItems.Add(new MenuItem { Icon = ((char)Symbol.Play).ToString(), Title = "New Game", PageType = typeof(NewGame) });
            vm.MenuItems.Add(new MenuItem { Icon = ((char)Symbol.People).ToString(), Title = "Internet Game", PageType = typeof(InternetGame) });
            vm.MenuItems.Add(new MenuItem { Icon = ((char)Symbol.Setting).ToString(), Title = "Settings", PageType = typeof(Options) });
            vm.MenuItems.Add(new MenuItem { Icon = ((char)Symbol.Help).ToString(), Title = "Help", PageType = typeof(HelpPage) });

            // select the first menu item
            vm.SelectedMenuItem = vm.MenuItems.First();


            this.ViewModel = vm;

            // add entry animations
            var transitions = new TransitionCollection { };
            var transition = new NavigationThemeTransition { };
            transitions.Add(transition);
            this.Frame.ContentTransitions = transitions;
            
        }

        public ShellViewModel ViewModel { get; private set; }

        public Frame RootFrame
        {
            get
            {
                return this.Frame;
            }
        }
    }
}
