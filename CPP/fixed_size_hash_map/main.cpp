#include "fixed_size_hash_map.h"

#include <iostream>
#include <cassert>
#include <string>
#include <vector>

using namespace std;

class StringComparator
{
public:
    bool operator()(const std::string& lhs, const std::string& rhs)
    {
        return lhs.compare(rhs) == 0;
    }
};

int main() {
    cout << "Begin testing" << endl;
    cout << "---------------" << endl;

    //constructing
    FixSizeHashMap<string, string, StringComparator> hm1(8);
    auto cmp = StringComparator();

    //removing a key that isn't in the map
    assert(hm1.remove("this key isn't in the map", cmp) == false);
    
    //emplacing a pair
    string world1 = "world1";
    assert(hm1.insert("hello1", world1, cmp));
    assert(hm1.get("hello1", cmp, world1) == true);
    assert(world1 == "world1");

    string world2 = "world2";
    assert(hm1.insert("hello2", world2, cmp));
    assert(hm1.get("hello2", cmp, world2) == true);
    assert(world2 == "world2");

    string world3 = "world3";
    assert(hm1.insert("hello3", world3, cmp));
    assert(hm1.get("hello3", cmp, world3) == true);
    assert(world3 == "world3");

    string world4 = "world4";
    assert(hm1.insert("hello4", world4, cmp));
    assert(hm1.get("hello4", cmp, world4) == true);
    assert(world4 == "world4");

    string world5 = "world5";
    assert(hm1.insert("hello5", world5, cmp));
    assert(hm1.get("hello5", cmp, world5) == true);
    assert(world5 == "world5");

    string world6 = "world6";
    assert(hm1.insert("hello6", world6, cmp));
    assert(hm1.get("hello6", cmp, world6) == true);
    assert(world6 == "world6");

    string world7 = "world7";
    assert(hm1.insert("hello7", world7, cmp));
    assert(hm1.get("hello7", cmp, world7) == true);
    assert(world7 == "world7");

    string world8 = "world8";
    assert(hm1.insert("hello8", world8, cmp));
    assert(hm1.get("hello8", cmp, world8) == true);
    assert(world8 == "world8");

    string world9 = "world9";
    assert(hm1.insert("hello9", world9, cmp) == false);

    assert(hm1.remove("hello8", cmp));

    assert(hm1.insert("hello9", world9, cmp));
    assert(hm1.get("hello9", cmp, world9) == true);
    assert(world9 == "world9");
    assert(hm1.remove("hello9", cmp));

    std::vector<string> total;
    hm1.get_all(total);
    assert(total.size() == 7);
    for (auto&& str: total) {
        cout << str << endl;
    }
    cout << "---------------" << endl;
    cout << "All tests passed!" << endl;
    return 0;
}