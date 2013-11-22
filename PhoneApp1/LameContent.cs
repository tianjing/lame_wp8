using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using Windows.Storage.Streams;

namespace PhoneApp1
{
    public static class LameContent
    {
        public const int Mode_STEREO = 0;
        public const int Mode_JOINT_STEREO =1;
        public const int Mode_MONO = 3;
        public const int Mode_NOT_SET = 4;

        public const int Quality_Best = 0;
        public const int Quality_NearBest = 1;
        public const int Quality_Better = 2;
        public const int Quality_NearBetter = 3;
        public const int Quality_Well = 4;
        public const int Quality_Middle = 5;
        public const int Quality_NearWorse = 6;
        public const int Quality_Worse = 7;
        public const int Quality_NearWorst = 8;
        public const int Quality_Worst = 9;
    }
}
