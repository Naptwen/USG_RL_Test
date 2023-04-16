# USG_RL_Test
This project is test for model engineering, MLops, Massive Multi cloud service for RL algorithm

# License GNU 3.0 Affero
# No Robots allow
# Image Source
- From OpenGame Art site

# test game example
- SDL2.0
- Socket TCP/IP

![test](https://user-images.githubusercontent.com/47798805/230782492-e7639f0b-4253-4cdb-b171-42050b61c4ff.gif)

# example for Table data function
```cpp
#include "BaseTable.h"
int main(int argc, char** argv) {
	BaseTable base_table;

    DATA mydata;  // make new data
    mydata += {"name", std::string("Tommy")}; 
    mydata += {"age", (int)42};
    mydata += {"size", (double)182.5f}; //No float is allow only double
    
    DATA mydata2; // make new data
    mydata2 += {"name", std::string("Amy")};
    mydata2 += {"age", (int)22};
    mydata2 += {"size", (double)168.0f};

    base_table.Add<std::string, int, double>(mydata); // add new data to table
    base_table.Add<std::string, int, double>(mydata2);// add new data to table
    auto col = base_table[{"name", "age"}]; //get cols that have name and age
    auto rows = base_table[col]; //get rows from columns
    for (auto r : rows) {
        std::cout << (*r).get<int>("age"); //Show all age data
    }
    base_table.ShowTable(); //Show all data in string
}
```
