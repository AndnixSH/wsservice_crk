using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using System.Xml;
using System.IO;

namespace TokensExtractor
{
    public partial class LicenseViewForm : Form
    {
        public XmlDocument Lic;

        public LicenseViewForm()
        {
            InitializeComponent();
        }

        private void buttonClose_Click(object sender, EventArgs e)
        {
            Close();
        }

        private XmlDocument LicOem;

        private void buttonOEMConv_Click(object sender, EventArgs e)
        {
            LicOem = Program.ConvertLicToOEM(Lic);
            if (LicOem != null)
            {
                textLic.Text = Program.FormatXml(LicOem);
                buttonOEMSave.Enabled = true;
            }
        }

        private void LicenseViewForm_Load(object sender, EventArgs e)
        {
            textLic.Text = Program.FormatXml(Lic);
        }

        private void SaveLicFile(string s)
        {
            SaveFileDialog saveFileDialog1 = new SaveFileDialog();

            saveFileDialog1.Filter = "lic files (*.lic)|*.lic|All files (*.*)|*.*";
            saveFileDialog1.FilterIndex = 0;
            saveFileDialog1.RestoreDirectory = true;

            if (saveFileDialog1.ShowDialog() == DialogResult.OK)
            {
                try
                {
                    File.WriteAllText(saveFileDialog1.FileName, s);
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message);
                }
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            SaveLicFile(LicOem.InnerXml);
        }

        private void buttonSaveOrig_Click(object sender, EventArgs e)
        {
            SaveLicFile(Lic.InnerXml);
        }
    }
}
