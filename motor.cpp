#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>


using namespace std;

//--------------------------------------------------------------------

// x = (2/3)*a
// y = (-1/3) * a + tan(30)*b;
// z = (-1/3) * a - tan(30)*b;


//--------------------------------------------------------------------


class	tiptilt {
public:;

		tiptilt();
		~tiptilt();

	void	Move(float d_alpha, float d_beta);
	void	MoveTo(float alpha, float beta);	
	void	Reset();

	void	MoveFocus(float delta);
	void	MoveToFocus(float value);	

private:
	int	InitUSB();
	void	CloseUSB();

	int	WriteCommand(string cmd);
	string	ReadResult();
};



//--------------------------------------------------------------------

	tiptilt::tiptilt()
{
}


//--------------------------------------------------------------------


	tiptilt::~tiptilt()
{
}

//--------------------------------------------------------------------

void	tiptilt::Move(float delta_alpha, float delta_beta)
{
}

//--------------------------------------------------------------------

void	tiptilt::MoveTo(float alpha, float beta)
{
}


//--------------------------------------------------------------------


void	tiptilt::MoveFocus(float delta)
{
}

//--------------------------------------------------------------------

void	tiptilt::MoveToFocus(float value)
{
}

//--------------------------------------------------------------------

int	tiptilt::InitUSB()
{
	return 0;
}

//--------------------------------------------------------------------

void	tiptilt::CloseUSB()
{
}

//--------------------------------------------------------------------

int	tiptilt::WriteCommand(string cmd)
{
	return 0;
}


//--------------------------------------------------------------------

string	tiptilt::ReadResult()
{
	return "";
}

//--------------------------------------------------------------------


int main()
{
	return 0;
}
