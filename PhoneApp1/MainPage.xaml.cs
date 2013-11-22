using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using PhoneApp1.Resources;
using Microsoft.Xna.Framework.Audio;
using Windows.Storage;
using System.IO;
using Windows.Foundation;
using Windows.Storage.Streams;
using System.Threading.Tasks;
using Lame_WP8;
using Microsoft.Phone.Storage;
using System.IO.IsolatedStorage;
using Windows.ApplicationModel;
using Microsoft.Xna.Framework.Media;
using Microsoft.Xna.Framework.Media.PhoneExtensions;
namespace PhoneApp1
{
    public partial class MainPage : PhoneApplicationPage
    {
        // 构造函数
        public MainPage()
        {
            InitializeComponent();
            mediaElement = new MediaElement();
            mediaElement.MediaFailed += Play_Error;
            LayoutRoot.Children.Add(mediaElement);
            // 用于本地化 ApplicationBar 的示例代码
            //BuildLocalizedApplicationBar();
        }
        MediaElement mediaElement;
        String m_LocalPath = Windows.Storage.ApplicationData.Current.LocalFolder.Path;
        String m_InstallPath = Windows.ApplicationModel.Package.Current.InstalledLocation.Path;
        private async void Button_WavePlay_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                String sourceFile = "xyy.mp3";
                String targetFile = "xyy.wave";
                IBuffer waveDates = await ReadWave("Assets/" + sourceFile);

                CompressedContent data = await Mp3ToWave(waveDates);
                byte[] mp3Data = data.Data;
                DeleteFile(targetFile);
                SaveFile(data.Data, targetFile);

                SoundEffect soudplay = new SoundEffect(mp3Data, 44100, AudioChannels.Stereo);
                soudplay.Play();

            }
            catch (Exception ex)
            {
                String s = ex.Message;
            }
        }
        private async void Button_Click(object sender, RoutedEventArgs e)
        {
            String sourceFile = "Assets/xyy.wav";
            String targetFile = "xyy.mp3";
            try
            {
                IBuffer waveDates = await ReadWave(sourceFile);

                CompressedContent data = await WaveToMp3(waveDates);
                byte[] mp3Data = data.Data;
                DeleteFile(targetFile);
                SaveFile(data.Data, targetFile);

                IsolatedStorageFileStream stream = new IsolatedStorageFileStream(targetFile, FileMode.Open, IsolatedStorageFile.GetUserStoreForApplication());
                mediaElement.SetSource(stream);
                mediaElement.Position = new TimeSpan(0);
                mediaElement.Play();
            }
            catch (Exception ex)
            {
                String s = ex.Message;
            }


        }


        private async void Button_Mp3FilePlay_Click(object sender, RoutedEventArgs e)
        {
            String sourceFile = "\\Assets\\xyy.wav";
            String targetFile = "\\xyy.mp3";
            try
            {

                Lame_WP8.LameWrapper mp3coder = GetLameWrapper();

                await mp3coder.EncodeMp3(m_InstallPath + sourceFile, m_LocalPath + targetFile);
            }
            catch (Exception ex)
            {
                String d = ex.Message;
            }
            using (IsolatedStorageFileStream stream = new IsolatedStorageFileStream(targetFile, FileMode.Open, IsolatedStorageFile.GetUserStoreForApplication()))
            {
                mediaElement.SetSource(stream);
                mediaElement.Position = new TimeSpan(0);
                mediaElement.Play();
            }

        }

        private async void Button_WavFilePlay_Click(object sender, RoutedEventArgs e)
        {
            String sourceFile = "\\Assets\\xyy.mp3";
            String targetFile = "\\xyy.wav";

            try
            {

                Lame_WP8.LameWrapper mp3coder = GetLameWrapper();

                await mp3coder.DecodeMp3(m_InstallPath + sourceFile, m_LocalPath + targetFile);
            }
            catch (Exception ex)
            {
                String d = ex.Message;
            }


            using (FileStream stream = File.Open(m_LocalPath+targetFile, FileMode.Open))
            {
                byte[] res = new byte[stream.Length];
                stream.Read(res, 0, res.Length);
                SoundEffect soudplay = new SoundEffect(res, 44100, AudioChannels.Stereo);
                stream.Close();
                soudplay.Play();

            }

        }



        private byte[] ReadFile(String p_FileName)
        {
            using (Stream stream = Microsoft.Xna.Framework.TitleContainer.OpenStream(p_FileName))
            {
                int length = (int)stream.Length;

                BinaryReader ss = new BinaryReader(stream);
                return ss.ReadBytes(length);

            }

        }
        private void DeleteFile(String p_FileName)
        {
            if (IsolatedStorageFile.GetUserStoreForApplication().FileExists(p_FileName))
            {
                IsolatedStorageFile.GetUserStoreForApplication().DeleteFile(p_FileName);
            }
        }
        private void SaveFile(byte[] p_FileData, String p_FileName)
        {
            //MemoryStream ms = new MemoryStream(mp3Data);
            using (IsolatedStorageFileStream file = new IsolatedStorageFileStream(p_FileName, FileMode.OpenOrCreate, IsolatedStorageFile.GetUserStoreForApplication()))
            {
                file.Write(p_FileData, 0, p_FileData.Length);
                file.Flush();
                file.Close();
            }
        }

        private void Play_Error(Object sender, ExceptionRoutedEventArgs e)
        {
            String s = e.ErrorException.Message;
        }

        private LameWrapper GetLameWrapper()
        {
            Lame_WP8.LameWrapper mp3coder = new Lame_WP8.LameWrapper();
            mp3coder.EncodeBrate = 128;
            mp3coder.EncodeQuality = LameContent.Quality_Well;
            return mp3coder;
        }
        private Task<CompressedContent> WaveToMp3(IBuffer p_WaveData)
        {
            Lame_WP8.LameWrapper mp3coder = GetLameWrapper();

            return mp3coder.EncodeMp3(p_WaveData).AsTask();
        }
        private Task<CompressedContent> Mp3ToWave(IBuffer p_WaveData)
        {
            Lame_WP8.LameWrapper mp3coder = new Lame_WP8.LameWrapper();
            //mp3coder.Brate = 128;
            //mp3coder.InSampleRate = 0;
            //mp3coder.Mode = LameContent.Mode_STEREO;
            // mp3coder.NumChannels = 2;
            //mp3coder.Quality = LameContent.Quality_Well;

            return new Lame_WP8.LameWrapper().DecodeMp3(p_WaveData).AsTask();
        }
        private Task<IBuffer> ReadWave(String p_FileName)
        {
            String fileName = p_FileName;
            Stream stream = Microsoft.Xna.Framework.TitleContainer.OpenStream(fileName);
            IBuffer bu = new Windows.Storage.Streams.Buffer((uint)stream.Length);

            return stream.AsInputStream().ReadAsync(bu, (uint)stream.Length, InputStreamOptions.None).AsTask();
            // return d.GetResults();
            //  return bu;
        }
        private byte[] Mp3ToWave()
        {
            String fileName = "xyy.mp3";
            int curr = 0;
            using (MemoryStream ms = new MemoryStream())
            {
                using (Stream stream = Microsoft.Xna.Framework.TitleContainer.OpenStream(fileName))
                {
                    byte[] buffe = new byte[1024];
                    int length = 0;
                    while ((length = stream.Read(buffe, 0, 1024)) > 0)
                    {
                        ms.Write(buffe, curr, length);
                        curr += length;
                    }
                }
                return ms.ToArray();
            }
        }




        // 用于生成本地化 ApplicationBar 的示例代码
        //private void BuildLocalizedApplicationBar()
        //{
        //    // 将页面的 ApplicationBar 设置为 ApplicationBar 的新实例。
        //    ApplicationBar = new ApplicationBar();

        //    // 创建新按钮并将文本值设置为 AppResources 中的本地化字符串。
        //    ApplicationBarIconButton appBarButton = new ApplicationBarIconButton(new Uri("/Assets/AppBar/appbar.add.rest.png", UriKind.Relative));
        //    appBarButton.Text = AppResources.AppBarButtonText;
        //    ApplicationBar.Buttons.Add(appBarButton);

        //    // 使用 AppResources 中的本地化字符串创建新菜单项。
        //    ApplicationBarMenuItem appBarMenuItem = new ApplicationBarMenuItem(AppResources.AppBarMenuItemText);
        //    ApplicationBar.MenuItems.Add(appBarMenuItem);
        //}
    }
}