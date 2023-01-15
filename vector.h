#pragma once
#include <iostream>

using namespace std;

template <typename T>
class Vector
{

public:
	Vector(){
		realocate(2); // Allocates memory for 2 elements
	}

	~Vector(){
		delete[] data;
	}

	void push_back(T val){

		if (count >= allocated_size){
			allocated_size *= 2;
			realocate(allocated_size);
		}

		data[count] = val;
		count++;
	}

	T pop_back(){

		if (count == 0){
			cout << "Vector is empty" << endl;
		}
		else if (count <= allocated_size / 2){
			allocated_size = allocated_size / 2;
			realocate(allocated_size);
		}
		count--;
		return data[count];
	}

	T& operator[](int index){
		
		if (index >= count){
			cout << "Index out of range" << endl;
		}
		else{
			return data[index];
		}
	}

	int size() { return count;}

	void clear(){
		
		int size = count;
		for (int i = 0; i < size; i++){
			pop_back();
		}
	}

	void delete_at_index(int index){
		
		if (index < 0 || index >= count)
			return;

		if (count <= allocated_size / 2){
			allocated_size = allocated_size / 2;
			realocate(allocated_size);
		}
		count--;

		for (int i = index; i < count; i++){
			data[i] = data[i + 1];
		}
	}

	void add_to_front(T val){
		
		if (count >= allocated_size){
			allocated_size *= 2;
			realocate(allocated_size);
		}

		for (int i = count; i > 0; i--){
			data[i] = data[i - 1];
		}

		data[0] = val;
		count++;
	}

private:
	T* data = nullptr;
	int count = 0;
	int allocated_size = 2;

	void realocate(int new_allocated_size){

		T* new_data = new T[new_allocated_size];

		if (new_data){
			
			for (int i = 0; i < count; i++){
				new_data[i] = data[i];
			}
			delete[] data;
			data = new_data;
		}
		else{
			cout << "error while realocating memory" << endl;
			return;
		}
	}
};