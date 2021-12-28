namespace DYTEST {

class Data {
public:
	Data();
	~Data();
	void print();

private:
	int data_;
};

Data* get_data();
Data* get_data2();
}
