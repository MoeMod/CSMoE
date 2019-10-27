using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace Launcher.Presentation
{
    public abstract class NotifyPropertyChanged : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        protected void OnPropertyChanged([CallerMemberName]string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        public bool Set<T>(ref T storage, T value, [CallerMemberName()]string propertyName = null)
        {
            if (!object.Equals(storage, value)) {
                storage = value;
                OnPropertyChanged(propertyName);
                return true;
            }
            return false;
        }
    }
}
