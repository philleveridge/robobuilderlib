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
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
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
            // ServoSim
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(809, 264);
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
    }
}

