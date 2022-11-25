using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows.Forms;
using System.IO;

namespace TokensExtractor
{
    public partial class AppxURLs : Form
    {
        public AppxURLs()
        {
            InitializeComponent();
        }

        private void buttonCopyURL_Click(object sender, EventArgs e)
        {
            if (lvAppxURLs.SelectedItems.Count > 0)
                Clipboard.SetText(lvAppxURLs.SelectedItems[0].Text);
        }

        private void buttonDownload_Click(object sender, EventArgs e)
        {
            if (lvAppxURLs.SelectedItems.Count > 0)
                System.Diagnostics.Process.Start(lvAppxURLs.SelectedItems[0].Text);
        }

        private void buttonClose_Click(object sender, EventArgs e)
        {
            Close();
        }

        private struct AppxURL
        {
            public string URL;
            public string DLFolder;
        }
        private List<AppxURL> AppxURLList;
        private bool DigWuauservLog()
        {
            try
            {
                AppxURLList = new List<AppxURL>();

                Regex gAppxUrl = new Regex("http://[^ ]*.appx?[^ ]*");
                Regex gDLFolder = new Regex((Environment.GetEnvironmentVariable("SystemRoot") + "\\SoftwareDistribution\\[^ ]*\\").Replace("\\", "\\\\"));

                FileStream fs = new FileStream(Environment.GetEnvironmentVariable("SystemRoot") + "\\WindowsUpdate.log", FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
                using (StreamReader r = new StreamReader(fs))
                {
                    string line;
                    while ((line = r.ReadLine()) != null)
                    {
                        Match mUrl = gAppxUrl.Match(line);
                        if (mUrl.Success)
                        {
                            Match mDLFolder = gDLFolder.Match(line);
                            if (mDLFolder.Success)
                            {
                                AppxURL url = new AppxURL();
                                url.URL = mUrl.Groups[0].Value;
                                url.DLFolder = mDLFolder.Groups[0].Value;
                                AppxURLList.Add(url);
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("DigForAppxURLs : " + ex.Message);
                return false;
            }
            return true;
        }


        private void AppxURLs_Load(object sender, EventArgs e)
        {
            if (!DigWuauservLog())
            {
                Close();
                return;
            }
            foreach(AppxURL au in AppxURLList)
            {
                ListViewItem lv = new ListViewItem();
                lv.Text = au.URL;
                lv.SubItems.Add(au.DLFolder);
                lvAppxURLs.Items.Add(lv);
            }
            if (lvAppxURLs.Items.Count > 0)
            {
                lvAppxURLs.Items[lvAppxURLs.Items.Count - 1].Selected = true;
                lvAppxURLs.Items[lvAppxURLs.Items.Count - 1].EnsureVisible();
            }
        }

        private void lvAppxURLs_SelectedIndexChanged(object sender, EventArgs e)
        {
            buttonCopyURL.Enabled = true;
            buttonDownload.Enabled = true;
        }
    }
}
