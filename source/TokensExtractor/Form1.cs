using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using System.Xml;

using System.IO;

namespace TokensExtractor
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void RefreshContent(bool bBruteForce=false)
        {
            Program.ReadTokens(bBruteForce);
            FillLbPFM();
        }


        private void buttonView_Click(object sender, EventArgs e)
        {
            if (lbPFM.SelectedIndex >= 0)
            {
                LicenseViewForm frmLV = new LicenseViewForm();
                frmLV.Lic = Program.tokens_LIC[lbPFM.SelectedIndex];
                frmLV.ShowDialog();
            }
        }

        private void buttonClose_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void FillLbPFM()
        {
            int idxSel = lbPFM.SelectedIndex;
            lbPFM.Items.Clear();
            foreach (XmlDocument x in Program.tokens_LIC)
            {

                XmlNode nodeRoot = x.FirstChild;
                if (nodeRoot == null)
                    break;
                XmlNode nodeLicenseInfo = nodeRoot["LicenseInfo"];
                string LicType = "";
                if (nodeLicenseInfo != null)
                {
                    XmlAttribute LicTypeAttr = nodeLicenseInfo.Attributes["Type"];
                    if (LicTypeAttr != null)
                        LicType = LicTypeAttr.Value;
                }
                XmlNode nodeBinding = nodeRoot["Binding"];
                if (nodeBinding == null)
                    break;
                XmlNode nodePFM = nodeBinding["PFM"];
                if (nodePFM == null)
                    break;

                lbPFM.Items.Add(nodePFM.InnerText + " (" + LicType + ")");
            }
            lbPFM.SelectedIndex = idxSel >= lbPFM.Items.Count ? lbPFM.Items.Count-1 : idxSel;
            if (lbPFM.SelectedIndex == -1)
                ListSelChange();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            FillLbPFM();
        }

        private void buttonCrackIt_Click(object sender, EventArgs e)
        {
            XmlDocument Lic = Program.tokens_LIC[lbPFM.SelectedIndex];
            if (Program.CrackIt(Lic))
                RefreshContent();
        }

        private void lbPFM_SelectedIndexChanged(object sender, EventArgs e)
        {
            ListSelChange();
        }

        private void ListSelChange()
        {
            bool bEnabled = false;
            if (lbPFM.SelectedIndex >= 0)
            {
                XmlDocument Lic = Program.tokens_LIC[lbPFM.SelectedIndex];
                XmlNode nodeRoot = Lic.FirstChild;
                if (nodeRoot != null)
                {
                    XmlNode nodeLicenseInfo = nodeRoot["LicenseInfo"];
                    if (nodeLicenseInfo != null)
                    {
                        XmlAttribute LicTypeAttr = nodeLicenseInfo.Attributes["Type"];
                        if (LicTypeAttr != null)
                            if (LicTypeAttr.Value == "Trial")
                                bEnabled = true;
                    }
                }
            }
            buttonView.Enabled = lbPFM.SelectedIndex >= 0;
            buttonUninstLic.Enabled = lbPFM.SelectedIndex >= 0;
            buttonCrackIt.Enabled = bEnabled;
        }

        private void buttonDigWuauservLog_Click(object sender, EventArgs e)
        {
            AppxURLs a = new AppxURLs();
            a.ShowDialog();
        }

        private void buttonDelDLCache_Click(object sender, EventArgs e)
        {
            string folderpath = Environment.GetEnvironmentVariable("SystemRoot") + "\\SoftwareDistribution\\Download";
            if (DialogResult.Yes == MessageBox.Show("You are about to delete content of "+folderpath+".\nAre you sure ?", "Delete download cache", MessageBoxButtons.YesNo))
            {
                try
                {
                    Program.EmptyFolder(new DirectoryInfo(folderpath));
                    MessageBox.Show("Deleted","Delete download cache");
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message);
                }
            }
        }

        private void buttonInstLicFromFile_Click(object sender, EventArgs e)
        {
            OpenFileDialog OpenFileDialog1 = new OpenFileDialog();

            OpenFileDialog1.Filter = "lic files (*.lic)|*.lic|All files (*.*)|*.*";
            OpenFileDialog1.FilterIndex = 0;
            OpenFileDialog1.RestoreDirectory = true;

            if (OpenFileDialog1.ShowDialog() == DialogResult.OK)
            {
                try
                {
                    string lic = File.ReadAllText(OpenFileDialog1.FileName);
                    if (Program.InstLic(lic,false))
                    {
                        RefreshContent();
                        MessageBox.Show("Installed", "Install license from a file");
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message);
                }
            }
        }

        private void buttonRefresh_Click(object sender, EventArgs e)
        {
            RefreshContent();
        }
        private void buttonBruteDig_Click(object sender, EventArgs e)
        {
            RefreshContent(true);
        }

        private void buttonUninstLic_Click(object sender, EventArgs e)
        {
            if (lbPFM.SelectedIndex >= 0)
            {
                XmlDocument Lic = Program.tokens_LIC[lbPFM.SelectedIndex];
                if (DialogResult.Yes == MessageBox.Show("You are about to uninstall license for " + lbPFM.SelectedItem.ToString() + ".\nAre you sure ?", "Uninstall license", MessageBoxButtons.YesNo))
                {
                    if (Program.UninstLic(Lic))
                    {
                        RefreshContent();
                        MessageBox.Show("Uninstalled.");
                    }
                }
            }
        }

        private void buttonFixStorewuauthTokensCache_Click(object sender, EventArgs e)
        {
            string tokencachefile = Environment.GetEnvironmentVariable("SystemRoot") + "\\SoftwareDistribution\\Plugins\\7D5F3CBA-03DB-4BE5-B4B36DBED19A6833\\117CAB2D-82B1-4B5A-A08C-4D62DBEE7782.cache";
            try
            {
                File.Delete(tokencachefile);
                MessageBox.Show("Storewuauth token cache deleted. Remember : you can download only with valid microsoft license. If you try to download with bogus license you will get 0x8024600e error.");
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message,"Delete storewuauth token cache file");
            }
        }

        private void buttonSearch_Click(object sender, EventArgs e)
        {
            if (lbPFM.Items.Count>0)
            {
                SearchForm frmSearch = new SearchForm();
                if (frmSearch.ShowDialog(this) == DialogResult.OK)
                {
                    string searchtext = frmSearch.searchtext.ToLower();
                    int startidx = lbPFM.SelectedIndex < 0 ? 0 : lbPFM.SelectedIndex;
                    for (int i = 0; i < lbPFM.Items.Count; i++)
                    {
                        int idx = (startidx + i) % lbPFM.Items.Count;
                        string itemtext = lbPFM.Items[idx].ToString().ToLower();
                        if (itemtext.IndexOf(searchtext) >= 0)
                        {
                            lbPFM.SelectedIndex = idx;
                            lbPFM.Focus();
                            return;
                        }
                    }
                    MessageBox.Show("Not found","Search");
                }
            }
        }

    }
}
