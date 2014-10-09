namespace RobobuilderLib
{
    partial class servoUC
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

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.servo = new System.Windows.Forms.VScrollBar();
            this.red = new System.Windows.Forms.CheckBox();
            this.blue = new System.Windows.Forms.CheckBox();
            this.passive = new System.Windows.Forms.CheckBox();
            this.SuspendLayout();
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(1, 147);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(34, 13);
            this.label2.TabIndex = 5;
            this.label2.Text = "Val: 0";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(1, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(36, 13);
            this.label1.TabIndex = 4;
            this.label1.Text = "ID: 32";
            // 
            // servo
            // 
            this.servo.Cursor = System.Windows.Forms.Cursors.WaitCursor;
            this.servo.LargeChange = 1;
            this.servo.Location = new System.Drawing.Point(6, 30);
            this.servo.Maximum = 254;
            this.servo.Name = "servo";
            this.servo.Size = new System.Drawing.Size(15, 108);
            this.servo.TabIndex = 3;
            this.servo.UseWaitCursor = true;
            this.servo.ValueChanged += new System.EventHandler(this.servo_ValueChanged);
            // 
            // red
            // 
            this.red.AutoSize = true;
            this.red.BackColor = System.Drawing.Color.Transparent;
            this.red.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.red.ForeColor = System.Drawing.Color.OrangeRed;
            this.red.Location = new System.Drawing.Point(1, 163);
            this.red.Name = "red";
            this.red.Size = new System.Drawing.Size(12, 11);
            this.red.TabIndex = 6;
            this.red.UseVisualStyleBackColor = false;
            // 
            // blue
            // 
            this.blue.AutoSize = true;
            this.blue.BackColor = System.Drawing.Color.Transparent;
            this.blue.Checked = true;
            this.blue.CheckState = System.Windows.Forms.CheckState.Checked;
            this.blue.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.blue.ForeColor = System.Drawing.Color.RoyalBlue;
            this.blue.Location = new System.Drawing.Point(19, 163);
            this.blue.Name = "blue";
            this.blue.Size = new System.Drawing.Size(12, 11);
            this.blue.TabIndex = 7;
            this.blue.UseVisualStyleBackColor = false;
            // 
            // passive
            // 
            this.passive.AutoSize = true;
            this.passive.Location = new System.Drawing.Point(6, 16);
            this.passive.Name = "passive";
            this.passive.Size = new System.Drawing.Size(15, 14);
            this.passive.TabIndex = 8;
            this.passive.UseVisualStyleBackColor = true;
            this.passive.CheckedChanged += new System.EventHandler(this.passive_CheckedChanged);
            // 
            // servoUC
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(255)))), ((int)(((byte)(192)))));
            this.Controls.Add(this.passive);
            this.Controls.Add(this.blue);
            this.Controls.Add(this.red);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.servo);
            this.Name = "servoUC";
            this.Size = new System.Drawing.Size(35, 180);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.VScrollBar servo;
        private System.Windows.Forms.CheckBox red;
        private System.Windows.Forms.CheckBox blue;
        private System.Windows.Forms.CheckBox passive;
    }
}
