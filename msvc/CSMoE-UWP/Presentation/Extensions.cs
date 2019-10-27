using System;
using System.Collections.Generic;
using System.Linq;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Media.Animation;

namespace Launcher
{
    public static class Extensions
    {
        public static List<Control> AllChildren(this DependencyObject parent)
        {
            var list = new List<Control>();

            for (int i = 0; i < VisualTreeHelper.GetChildrenCount(parent); i++)
            {
                var _Child = VisualTreeHelper.GetChild(parent, i);
                if (_Child is Control)
                {
                    list.Add(_Child as Control);
                }
                list.AddRange(AllChildren(_Child));
            }

            return list;
        }

        public static T GetChild<T>(this DependencyObject parentContainer, string controlName)
        {
            var childControls = AllChildren(parentContainer);
            var control = childControls.OfType<Control>().Where(x => x.Name.Equals(controlName)).Cast<T>().First();

            return control;
        }

        public static T GetParent<T>(this FrameworkElement element, string message = null) where T : DependencyObject
        {
            var parent = element.Parent as T;

            if (parent == null)
            {
                if (message == null)
                {
                    message = "Parent element should not be null! Check the default Generic.xaml.";
                }

                throw new NullReferenceException(message);
            }

            return parent;
        }

        public static T GetChild<T>(this Border element, string message = null) where T : DependencyObject
        {
            var child = element.Child as T;

            if (child == null)
            {
                if (message == null)
                {
                    message = $"{nameof(Border)}'s child should not be null! Check the default Generic.xaml.";
                }

                throw new NullReferenceException(message);
            }

            return child;
        }

        public static Storyboard GetStoryboard(this FrameworkElement element, string name, string message = null)
        {
            var storyboard = element.Resources[name] as Storyboard;

            if (storyboard == null)
            {
                if (message == null)
                {
                    message = $"Storyboard '{name}' cannot be found! Check the default Generic.xaml.";
                }

                throw new NullReferenceException(message);
            }

            return storyboard;
        }

        public static CompositeTransform GetCompositeTransform(this FrameworkElement element, string message = null)
        {
            var transform = element.RenderTransform as CompositeTransform;

            if (transform == null)
            {
                if (message == null)
                {
                    message = $"{element.Name}'s RenderTransform should be a CompositeTransform! Check the default Generic.xaml.";
                }

                throw new NullReferenceException(message);
            }

            return transform;
        }
    }
}
