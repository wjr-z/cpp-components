#include "random.hpp"

#include <wjr/container/btree_map.hpp>
#include <wjr/container/vector.hpp>

using namespace wjr;

TEST(btree_map, construct) {
    {
        auto test = [](auto key, auto value) {
            using key_type = remove_cvref_t<decltype(key)>;
            using value_type = remove_cvref_t<decltype(value)>;
            {
                btree_map<key_type, value_type> map;
                WJR_ASSERT_L0(map.empty());
                WJR_ASSERT_L0(map.size() == 0);
                WJR_ASSERT_L0(map.begin() == map.end());
                WJR_ASSERT_L0(map.rbegin() == map.rend());
            }

            for (int n = 0; n < 1024; n = (n << 1) | 1) {
                vector<std::pair<key_type, value_type>> vec;
                for (int i = 0; i < n; ++i) {
                    vec.emplace_back(trandom<key_type>(), trandom<value_type>());
                }

                btree_map<key_type, value_type> map(vec.begin(), vec.end());
                std::map<key_type, value_type> std_map(vec.begin(), vec.end());
                WJR_ASSERT_L0(std::equal(map.begin(), map.end(), std_map.begin(), std_map.end()));
                btree_map<key_type, value_type> map2(map);
                WJR_ASSERT_L0(std::equal(map2.begin(), map2.end(), std_map.begin(), std_map.end()));
                WJR_ASSERT_L0(std::equal(map.begin(), map.end(), map2.begin(), map2.end()));
                btree_map<key_type, value_type> map3(std::move(map2));
                WJR_ASSERT_L0(map2.empty());
                WJR_ASSERT_L0(std::equal(map3.begin(), map3.end(), std_map.begin(), std_map.end()));
                WJR_ASSERT_L0(std::equal(map.begin(), map.end(), map3.begin(), map3.end()));
            }
        };

        test(int(0), int(0));
        test(std::string(), std::string());
    }
}

TEST(btree_map, emplace) {
    {
        auto test = [](auto key, auto value) {
            using key_type = remove_cvref_t<decltype(key)>;
            using value_type = remove_cvref_t<decltype(value)>;
            for (int n = 0; n < 1024; n = (n << 1) | 1) {
                vector<std::pair<key_type, value_type>> vec;
                for (int i = 0; i < n; ++i) {
                    vec.emplace_back(trandom<key_type>(), trandom<value_type>());
                }

                {
                    btree_map<key_type, value_type> map;
                    std::map<key_type, value_type> std_map;

                    for (const auto &pr : vec) {
                        map.emplace(pr);
                        std_map.emplace(pr);
                    }

                    WJR_ASSERT_L0(
                        std::equal(map.begin(), map.end(), std_map.begin(), std_map.end()));
                }

                {
                    btree_map<key_type, value_type> map;
                    std::map<key_type, value_type> std_map;

                    for (const auto &pr : vec) {
                        map.emplace(pr.first, pr.second);
                        std_map.emplace(pr.first, pr.second);
                    }

                    WJR_ASSERT_L0(
                        std::equal(map.begin(), map.end(), std_map.begin(), std_map.end()));
                }
            }
        };

        test(int(0), int(0));
        test(std::string(), std::string());
    }
}