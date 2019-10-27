using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Launcher.Presentation
{
    public class MenuItem : NotifyPropertyChanged
    {
        private string icon;
        private string title;
        private Type pageType;


        public string Icon
        {
            get { return this.icon; }
            set { Set(ref this.icon, value); }
        }

        public string Title
        {
            get { return this.title; }
            set { Set(ref this.title, value); }
        }

        public Type PageType
        {
            get { return this.pageType; }
            set { Set(ref this.pageType, value); }
        }
    }
}
