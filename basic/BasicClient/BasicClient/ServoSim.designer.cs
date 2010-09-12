namespace RobobuilderLib
{
    partial class ServoSim
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ServoSim));
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.button1 = new System.Windows.Forms.Button();
            this.xv = new System.Windows.Forms.VScrollBar();
            this.yv = new System.Windows.Forms.VScrollBar();
            this.zv = new System.Windows.Forms.VScrollBar();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.psdv = new System.Windows.Forms.VScrollBar();
            this.label4 = new System.Windows.Forms.Label();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.servoIter = new System.Windows.Forms.Button();
            this.pictureBox2 = new System.Windows.Forms.PictureBox();
            this.pf1_btn = new System.Windows.Forms.Button();
            this.pf2_btn = new System.Windows.Forms.Button();
            this.checkBox1 = new System.Windows.Forms.CheckBox();
            this.checkBox2 = new System.Windows.Forms.CheckBox();
            this.checkBox3 = new System.Windows.Forms.CheckBox();
            this.checkBox4 = new System.Windows.Forms.CheckBox();
            this.checkBox5 = new System.Windows.Forms.CheckBox();
            this.checkBox6 = new System.Windows.Forms.CheckBox();
            this.checkBox7 = new System.Windows.Forms.CheckBox();
            this.checkBox8 = new System.Windows.Forms.CheckBox();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox2)).BeginInit();
            this.SuspendLayout();
            // 
            // textBox1
            // 
            this.textBox1.Location = new System.Drawing.Point(5, 236);
            this.textBox1.Multiline = true;
            this.textBox1.Name = "textBox1";
            this.textBox1.Size = new System.Drawing.Size(263, 27);
            this.textBox1.TabIndex = 3;
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(587, 236);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(53, 29);
            this.button1.TabIndex = 4;
            this.button1.Text = "Go!";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // xv
            // 
            this.xv.Location = new System.Drawing.Point(543, 24);
            this.xv.Maximum = 20;
            this.xv.Minimum = -20;
            this.xv.Name = "xv";
            this.xv.Size = new System.Drawing.Size(28, 79);
            this.xv.TabIndex = 5;
            this.xv.ValueChanged += new System.EventHandler(this.ValueChanged);
            // 
            // yv
            // 
            this.yv.Location = new System.Drawing.Point(571, 24);
            this.yv.Maximum = 20;
            this.yv.Minimum = -20;
            this.yv.Name = "yv";
            this.yv.Size = new System.Drawing.Size(28, 79);
            this.yv.TabIndex = 6;
            this.yv.ValueChanged += new System.EventHandler(this.ValueChanged);
            // 
            // zv
            // 
            this.zv.Location = new System.Drawing.Point(599, 24);
            this.zv.Maximum = 20;
            this.zv.Minimum = -20;
            this.zv.Name = "zv";
            this.zv.Size = new System.Drawing.Size(28, 79);
            this.zv.TabIndex = 7;
            this.zv.ValueChanged += new System.EventHandler(this.ValueChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(557, 112);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(14, 13);
            this.label1.TabIndex = 8;
            this.label1.Text = "X";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(585, 112);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(14, 13);
            this.label2.TabIndex = 9;
            this.label2.Text = "Y";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(613, 112);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(14, 13);
            this.label3.TabIndex = 10;
            this.label3.Text = "Z";
            // 
            // psdv
            // 
            this.psdv.Location = new System.Drawing.Point(543, 140);
            this.psdv.Maximum = 50;
            this.psdv.Minimum = 10;
            this.psdv.Name = "psdv";
            this.psdv.Size = new System.Drawing.Size(30, 74);
            this.psdv.TabIndex = 11;
            this.psdv.Value = 10;
            this.psdv.ValueChanged += new System.EventHandler(this.ValueChanged);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(544, 214);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(29, 13);
            this.label4.TabIndex = 12;
            this.label4.Text = "PSD";
            // 
            // pictureBox1
            // 
            this.pictureBox1.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox1.Image")));
            this.pictureBox1.Location = new System.Drawing.Point(646, -6);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(163, 271);
            this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.pictureBox1.TabIndex = 37;
            this.pictureBox1.TabStop = false;
            this.pictureBox1.MouseClick += new System.Windows.Forms.MouseEventHandler(this.pictureBox1_Click);
            // 
            // servoIter
            // 
            this.servoIter.Location = new System.Drawing.Point(508, 7);
            this.servoIter.Name = "servoIter";
            this.servoIter.Size = new System.Drawing.Size(22, 21);
            this.servoIter.TabIndex = 38;
            this.servoIter.Text = "0";
            this.servoIter.UseVisualStyleBackColor = true;
            this.servoIter.Click += new System.EventHandler(this.servoIter_Click);
            // 
            // pictureBox2
            // 
            this.pictureBox2.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox2.Image")));
            this.pictureBox2.Location = new System.Drawing.Point(827, 6);
            this.pictureBox2.Name = "pictureBox2";
            this.pictureBox2.Size = new System.Drawing.Size(160, 106);
            this.pictureBox2.TabIndex = 39;
            this.pictureBox2.TabStop = false;
            // 
            // pf1_btn
            // 
            this.pf1_btn.Location = new System.Drawing.Point(960, 39);
            this.pf1_btn.Name = "pf1_btn";
            this.pf1_btn.Size = new System.Drawing.Size(37, 18);
            this.pf1_btn.TabIndex = 40;
            this.pf1_btn.Text = "PF1";
            this.pf1_btn.UseVisualStyleBackColor = true;
            this.pf1_btn.Click += new System.EventHandler(this.pf1_btn_Click);
            // 
            // pf2_btn
            // 
            this.pf2_btn.Location = new System.Drawing.Point(960, 63);
            this.pf2_btn.Name = "pf2_btn";
            this.pf2_btn.Size = new System.Drawing.Size(37, 18);
            this.pf2_btn.TabIndex = 41;
            this.pf2_btn.Text = "PF2";
            this.pf2_btn.UseVisualStyleBackColor = true;
            this.pf2_btn.Click += new System.EventHandler(this.pf2_btn_Click);
            // 
            // checkBox1
            // 
            this.checkBox1.AutoSize = true;
            this.checkBox1.BackColor = System.Drawing.Color.White;
            this.checkBox1.Checked = true;
            this.checkBox1.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBox1.FlatAppearance.BorderColor = System.Drawing.Color.Green;
            this.checkBox1.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.checkBox1.ForeColor = System.Drawing.Color.Red;
            this.checkBox1.Location = new System.Drawing.Point(844, 146);
            this.checkBox1.Name = "checkBox1";
            this.checkBox1.Size = new System.Drawing.Size(50, 17);
            this.checkBox1.TabIndex = 42;
            this.checkBox1.Text = "RUN ";
            this.checkBox1.UseVisualStyleBackColor = false;
            // 
            // checkBox2
            // 
            this.checkBox2.AutoSize = true;
            this.checkBox2.Checked = true;
            this.checkBox2.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBox2.FlatAppearance.BorderColor = System.Drawing.Color.Blue;
            this.checkBox2.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.checkBox2.ForeColor = System.Drawing.Color.RoyalBlue;
            this.checkBox2.Location = new System.Drawing.Point(827, 148);
            this.checkBox2.Name = "checkBox2";
            this.checkBox2.Size = new System.Drawing.Size(12, 11);
            this.checkBox2.TabIndex = 43;
            this.checkBox2.UseVisualStyleBackColor = true;
            // 
            // checkBox3
            // 
            this.checkBox3.AutoSize = true;
            this.checkBox3.Checked = true;
            this.checkBox3.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBox3.FlatAppearance.BorderColor = System.Drawing.Color.Red;
            this.checkBox3.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.checkBox3.ForeColor = System.Drawing.Color.Red;
            this.checkBox3.Location = new System.Drawing.Point(844, 124);
            this.checkBox3.Name = "checkBox3";
            this.checkBox3.Size = new System.Drawing.Size(49, 17);
            this.checkBox3.TabIndex = 44;
            this.checkBox3.Text = "PWR";
            this.checkBox3.UseVisualStyleBackColor = true;
            // 
            // checkBox4
            // 
            this.checkBox4.AutoSize = true;
            this.checkBox4.Checked = true;
            this.checkBox4.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBox4.FlatAppearance.BorderColor = System.Drawing.Color.Green;
            this.checkBox4.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.checkBox4.ForeColor = System.Drawing.Color.Green;
            this.checkBox4.Location = new System.Drawing.Point(827, 127);
            this.checkBox4.Name = "checkBox4";
            this.checkBox4.Size = new System.Drawing.Size(12, 11);
            this.checkBox4.TabIndex = 45;
            this.checkBox4.UseVisualStyleBackColor = true;
            // 
            // checkBox5
            // 
            this.checkBox5.AutoSize = true;
            this.checkBox5.Checked = true;
            this.checkBox5.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBox5.FlatAppearance.BorderColor = System.Drawing.Color.Red;
            this.checkBox5.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.checkBox5.ForeColor = System.Drawing.Color.Red;
            this.checkBox5.Location = new System.Drawing.Point(844, 169);
            this.checkBox5.Name = "checkBox5";
            this.checkBox5.Size = new System.Drawing.Size(62, 17);
            this.checkBox5.TabIndex = 46;
            this.checkBox5.Text = "ERROR";
            this.checkBox5.UseVisualStyleBackColor = true;
            // 
            // checkBox6
            // 
            this.checkBox6.AutoSize = true;
            this.checkBox6.Checked = true;
            this.checkBox6.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBox6.FlatAppearance.BorderColor = System.Drawing.Color.Red;
            this.checkBox6.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.checkBox6.ForeColor = System.Drawing.Color.Red;
            this.checkBox6.Location = new System.Drawing.Point(952, 121);
            this.checkBox6.Name = "checkBox6";
            this.checkBox6.Size = new System.Drawing.Size(42, 17);
            this.checkBox6.TabIndex = 47;
            this.checkBox6.Text = "PF1";
            this.checkBox6.UseVisualStyleBackColor = true;
            // 
            // checkBox7
            // 
            this.checkBox7.AutoSize = true;
            this.checkBox7.Checked = true;
            this.checkBox7.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBox7.FlatAppearance.BorderColor = System.Drawing.Color.Blue;
            this.checkBox7.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.checkBox7.ForeColor = System.Drawing.Color.Blue;
            this.checkBox7.Location = new System.Drawing.Point(934, 124);
            this.checkBox7.Name = "checkBox7";
            this.checkBox7.Size = new System.Drawing.Size(12, 11);
            this.checkBox7.TabIndex = 48;
            this.checkBox7.UseVisualStyleBackColor = true;
            // 
            // checkBox8
            // 
            this.checkBox8.AutoSize = true;
            this.checkBox8.Checked = true;
            this.checkBox8.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBox8.FlatAppearance.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(128)))), ((int)(((byte)(0)))));
            this.checkBox8.FlatAppearance.CheckedBackColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(128)))), ((int)(((byte)(0)))));
            this.checkBox8.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.checkBox8.ForeColor = System.Drawing.Color.Orange;
            this.checkBox8.Location = new System.Drawing.Point(952, 146);
            this.checkBox8.Name = "checkBox8";
            this.checkBox8.Size = new System.Drawing.Size(42, 17);
            this.checkBox8.TabIndex = 49;
            this.checkBox8.Text = "PF2";
            this.checkBox8.UseVisualStyleBackColor = true;
            // 
            // ServoSim
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1006, 264);
            this.Controls.Add(this.checkBox8);
            this.Controls.Add(this.checkBox7);
            this.Controls.Add(this.checkBox6);
            this.Controls.Add(this.checkBox5);
            this.Controls.Add(this.checkBox4);
            this.Controls.Add(this.checkBox3);
            this.Controls.Add(this.checkBox2);
            this.Controls.Add(this.checkBox1);
            this.Controls.Add(this.pf2_btn);
            this.Controls.Add(this.pf1_btn);
            this.Controls.Add(this.pictureBox2);
            this.Controls.Add(this.servoIter);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.psdv);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.zv);
            this.Controls.Add(this.yv);
            this.Controls.Add(this.xv);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.textBox1);
            this.Name = "ServoSim";
            this.Text = "RBC Simulator";
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox2)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }


        #endregion

        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.VScrollBar xv;
        private System.Windows.Forms.VScrollBar yv;
        private System.Windows.Forms.VScrollBar zv;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.VScrollBar psdv;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.Button servoIter;
        private System.Windows.Forms.PictureBox pictureBox2;
        private System.Windows.Forms.Button pf1_btn;
        private System.Windows.Forms.Button pf2_btn;
        private System.Windows.Forms.CheckBox checkBox1;
        private System.Windows.Forms.CheckBox checkBox2;
        private System.Windows.Forms.CheckBox checkBox3;
        private System.Windows.Forms.CheckBox checkBox4;
        private System.Windows.Forms.CheckBox checkBox5;
        private System.Windows.Forms.CheckBox checkBox6;
        private System.Windows.Forms.CheckBox checkBox7;
        private System.Windows.Forms.CheckBox checkBox8;
    }
}

