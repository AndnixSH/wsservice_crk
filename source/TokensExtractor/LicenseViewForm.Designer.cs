namespace TokensExtractor
{
    partial class LicenseViewForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.textLic = new System.Windows.Forms.TextBox();
            this.buttonOEMConv = new System.Windows.Forms.Button();
            this.buttonClose = new System.Windows.Forms.Button();
            this.buttonOEMSave = new System.Windows.Forms.Button();
            this.buttonSaveOrig = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // textLic
            // 
            this.textLic.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textLic.Location = new System.Drawing.Point(12, 12);
            this.textLic.Multiline = true;
            this.textLic.Name = "textLic";
            this.textLic.ReadOnly = true;
            this.textLic.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.textLic.Size = new System.Drawing.Size(673, 608);
            this.textLic.TabIndex = 0;
            this.textLic.WordWrap = false;
            // 
            // buttonOEMConv
            // 
            this.buttonOEMConv.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonOEMConv.Location = new System.Drawing.Point(699, 37);
            this.buttonOEMConv.Name = "buttonOEMConv";
            this.buttonOEMConv.Size = new System.Drawing.Size(113, 62);
            this.buttonOEMConv.TabIndex = 1;
            this.buttonOEMConv.Text = "Convert to OEM";
            this.buttonOEMConv.UseVisualStyleBackColor = true;
            this.buttonOEMConv.Click += new System.EventHandler(this.buttonOEMConv_Click);
            // 
            // buttonClose
            // 
            this.buttonClose.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonClose.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonClose.Location = new System.Drawing.Point(699, 209);
            this.buttonClose.Name = "buttonClose";
            this.buttonClose.Size = new System.Drawing.Size(113, 31);
            this.buttonClose.TabIndex = 4;
            this.buttonClose.Text = "&Close";
            this.buttonClose.UseVisualStyleBackColor = true;
            this.buttonClose.Click += new System.EventHandler(this.buttonClose_Click);
            // 
            // buttonOEMSave
            // 
            this.buttonOEMSave.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonOEMSave.Enabled = false;
            this.buttonOEMSave.Location = new System.Drawing.Point(699, 159);
            this.buttonOEMSave.Name = "buttonOEMSave";
            this.buttonOEMSave.Size = new System.Drawing.Size(112, 29);
            this.buttonOEMSave.TabIndex = 3;
            this.buttonOEMSave.Text = "Save OEM";
            this.buttonOEMSave.UseVisualStyleBackColor = true;
            this.buttonOEMSave.Click += new System.EventHandler(this.button1_Click);
            // 
            // buttonSaveOrig
            // 
            this.buttonSaveOrig.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonSaveOrig.Location = new System.Drawing.Point(699, 124);
            this.buttonSaveOrig.Name = "buttonSaveOrig";
            this.buttonSaveOrig.Size = new System.Drawing.Size(112, 29);
            this.buttonSaveOrig.TabIndex = 2;
            this.buttonSaveOrig.Text = "Save original";
            this.buttonSaveOrig.UseVisualStyleBackColor = true;
            this.buttonSaveOrig.Click += new System.EventHandler(this.buttonSaveOrig_Click);
            // 
            // LicenseViewForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonClose;
            this.ClientSize = new System.Drawing.Size(824, 630);
            this.Controls.Add(this.buttonSaveOrig);
            this.Controls.Add(this.buttonOEMSave);
            this.Controls.Add(this.buttonClose);
            this.Controls.Add(this.buttonOEMConv);
            this.Controls.Add(this.textLic);
            this.Name = "LicenseViewForm";
            this.Text = "License view";
            this.Load += new System.EventHandler(this.LicenseViewForm_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox textLic;
        private System.Windows.Forms.Button buttonOEMConv;
        private System.Windows.Forms.Button buttonClose;
        private System.Windows.Forms.Button buttonOEMSave;
        private System.Windows.Forms.Button buttonSaveOrig;
    }
}