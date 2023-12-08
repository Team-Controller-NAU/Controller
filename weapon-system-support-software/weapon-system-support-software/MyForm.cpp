#include "MyForm.h"

using namespace System;
using namespace System::Windows::Forms;

void main(array<String^>^ args)
{
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);
	weaponsystemsupportsoftware::MyForm form;
	Application::Run(% form);
}