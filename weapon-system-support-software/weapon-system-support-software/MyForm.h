#pragma once
#include "SerialCom.h"

namespace weaponsystemsupportsoftware {
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for MyForm
	/// </summary>
	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		MyForm(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~MyForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Label^ messagelb;
	protected:

	private: System::Windows::Forms::Label^ statuslb;
	private: System::Windows::Forms::Label^ sentlb;
	private: System::Windows::Forms::Label^ reclb;

	protected:

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->messagelb = (gcnew System::Windows::Forms::Label());
			this->statuslb = (gcnew System::Windows::Forms::Label());
			this->sentlb = (gcnew System::Windows::Forms::Label());
			this->reclb = (gcnew System::Windows::Forms::Label());
			this->SuspendLayout();
			// 
			// messagelb
			// 
			this->messagelb->AutoSize = true;
			this->messagelb->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->messagelb->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 32, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->messagelb->ForeColor = System::Drawing::Color::White;
			this->messagelb->Location = System::Drawing::Point(36, 190);
			this->messagelb->Name = L"messagelb";
			this->messagelb->Size = System::Drawing::Size(590, 53);
			this->messagelb->TabIndex = 0;
			this->messagelb->Text = L"Click here to send a message";
			this->messagelb->Click += gcnew System::EventHandler(this, &MyForm::messagelb_Click);
			// 
			// statuslb
			// 
			this->statuslb->AutoSize = true;
			this->statuslb->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 24.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->statuslb->ForeColor = System::Drawing::Color::White;
			this->statuslb->Location = System::Drawing::Point(27, 9);
			this->statuslb->Name = L"statuslb";
			this->statuslb->Size = System::Drawing::Size(297, 38);
			this->statuslb->TabIndex = 1;
			this->statuslb->Text = L"Connection status: ";
			this->statuslb->Click += gcnew System::EventHandler(this, &MyForm::statuslb_Click);
			// 
			// sentlb
			// 
			this->sentlb->AutoSize = true;
			this->sentlb->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 24.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->sentlb->ForeColor = System::Drawing::Color::White;
			this->sentlb->Location = System::Drawing::Point(29, 47);
			this->sentlb->Name = L"sentlb";
			this->sentlb->Size = System::Drawing::Size(244, 38);
			this->sentlb->TabIndex = 2;
			this->sentlb->Text = L"Sent Message: ";
			// 
			// reclb
			// 
			this->reclb->AutoSize = true;
			this->reclb->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 24.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->reclb->ForeColor = System::Drawing::Color::White;
			this->reclb->Location = System::Drawing::Point(27, 85);
			this->reclb->Name = L"reclb";
			this->reclb->Size = System::Drawing::Size(314, 38);
			this->reclb->TabIndex = 3;
			this->reclb->Text = L"Received Message: ";
			// 
			// MyForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::SystemColors::ActiveCaptionText;
			this->ClientSize = System::Drawing::Size(1258, 261);
			this->Controls->Add(this->reclb);
			this->Controls->Add(this->sentlb);
			this->Controls->Add(this->statuslb);
			this->Controls->Add(this->messagelb);
			this->Name = L"MyForm";
			this->Text = L"Team Controller Demo";
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void messagelb_Click(System::Object^ sender, System::EventArgs^ e) {
		// initialize output array that demo will produce
		std::string* output;

		// do serial demo
		output = doDemo();

		// extract sent/received messages from output
		String^ sentMessage = gcnew String(output[0].c_str());
		String^ receivedMessage = gcnew String(output[1].c_str());
		String^ connectionMessage = "The controller port and laptop port have been successfully opened.";

		// update gui message
		this->statuslb->Text = "Connection status: " + connectionMessage;
		this->sentlb->Text = "Sent Message: " + sentMessage;
		this->reclb->Text = "Received Message: " + receivedMessage;

		// free for every new
		delete[] sentMessage, receivedMessage, output;
	}
		   
	private: System::Void statuslb_Click(System::Object^ sender, System::EventArgs^ e) {
	}
	};
}
