using Microsoft.Xna.Framework;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Threading;

namespace PhoneApp1
{
    public  class XnaAsyncDispatcher : IApplicationService
    {
        private readonly DispatcherTimer _frameworkDispatcherTimer;
        public XnaAsyncDispatcher(TimeSpan dispatchInterval)
        {
            FrameworkDispatcher.Update();
            _frameworkDispatcherTimer = new DispatcherTimer();
            _frameworkDispatcherTimer.Tick += FrameworkDispatcherTimer_Tick;
            _frameworkDispatcherTimer.Interval = dispatchInterval;
        }
        void IApplicationService.StartService(ApplicationServiceContext context)
        {
            _frameworkDispatcherTimer.Start();
        }

        void IApplicationService.StopService()
        {
            _frameworkDispatcherTimer.Stop();
        }

        private static void FrameworkDispatcherTimer_Tick(object sender, EventArgs e)
        {
            FrameworkDispatcher.Update();
        }
    }
}
