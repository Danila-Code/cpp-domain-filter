#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <string_view>
#include <vector>

#include <cassert>

using namespace std::literals;

class Domain {
public:
    // для тестирование конструирования объекта Domain из string
    friend std::ostream& operator<<(std::ostream&, const Domain&);

    Domain(std::string_view domain_name) : domain_name_{std::string(domain_name)} {
    }

    bool operator==(const Domain& other) const noexcept {
        return domain_name_ == other.domain_name_;
    }

    // сравнивает имена доменов лексикографически, начиная с конца строки, более короткие домены считаются меньше длинных (.ru < .cru) 
    bool operator<(const Domain& other) const noexcept {
        return std::lexicographical_compare(domain_name_.rbegin(), domain_name_.rend(), 
            other.domain_name_.rbegin(), other.domain_name_.rend(),
            [](char l, char r) {
                return (l == '.' || l < r) && (r != '.');
        });
    }

    bool IsSubdomain(const Domain& other) const {
        return domain_name_.size() >= other.domain_name_.size() &&
               std::string_view('.' + domain_name_).ends_with("." + other.domain_name_);
    }
private:
    std::string domain_name_;
};

class DomainChecker {
public:
    // для тестирование конструирования объекта DomainChecker из двух итераторов
    friend std::ostream& operator<<(std::ostream&, const DomainChecker&);

    template <typename InputIter>
    DomainChecker(InputIter begin, InputIter end) : forbidden_domains_(begin, end) {
        PrepareForbiddenDomains();
    }

    bool IsForbidden(const Domain& domain) const {
        auto find_domain = std::upper_bound(forbidden_domains_.begin(), forbidden_domains_.end(), domain);

        return find_domain == forbidden_domains_.begin()
                                                         ? false
                                                         : domain.IsSubdomain(*(--find_domain));
    }
private:
    // сортирует вектор доменов, убирает дубликаты и лишние поддомены
    void PrepareForbiddenDomains() const {
        std::sort(forbidden_domains_.begin(), forbidden_domains_.end());

        auto new_end_iter = std::unique(forbidden_domains_.begin(), forbidden_domains_.end(), 
            [](const Domain& lhs, const Domain& rhs) {
                return lhs.IsSubdomain(rhs) || rhs.IsSubdomain(lhs);
        });
        forbidden_domains_.erase(new_end_iter, forbidden_domains_.end());
    }

    mutable std::vector<Domain> forbidden_domains_;
};

// Читаем number доменов из потока input
std::vector<Domain> ReadDomains(std::istream& input, const size_t number) {
    std::vector<Domain> domains;
    domains.reserve(number);
    if(!number) {
        return domains;
    }
    for(size_t i = 0; i < number; ++i) {
        std::string domain_name;
        getline(input, domain_name);
        domains.emplace_back(std::move(domain_name));
    }
    return domains;
}

template <typename Number>
Number ReadNumberOnLine(std::istream& input) {
    std::string line;
    getline(input, line);

    Number num;
    std::istringstream(line) >> num;

    return num;
}

// ********************************** Тесты *******************************************************
std::ostream& operator<<(std::ostream& out, const Domain& domain) {
    out << domain.domain_name_;
    return out;
}

std::ostream& operator<<(std::ostream& out, const std::vector<Domain>& domains) {
    for(const Domain& domain : domains) {
        out << domain << std::endl;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const DomainChecker& checker) {
    out << checker.forbidden_domains_;
    return out;
}

void TestDomain() {
    // тестирование конструктора и оператора ==
    {
        std::string s1("com"sv);
        std::string s2("ru"sv);
        std::ostringstream ostr_stream;
        
        Domain domain1(s1);
        ostr_stream << domain1;
        assert(ostr_stream.str() == s1);
        ostr_stream.str("");
        ostr_stream.clear();

        Domain domain2(s2);
        ostr_stream << domain2;
        assert(ostr_stream.str() == s2);
        assert(domain1 != domain2);
    }
    {
        std::string s1("com"sv);
        std::string s2("com"sv);
        Domain domain1(s1);
        Domain domain2(s2);
        assert(domain1 == domain2);
    }
    // тестирование IsSubdomain
    {
        Domain domain("com"sv);
        Domain subdomain("duck.com"sv);
        assert(subdomain.IsSubdomain(domain) == true);
        Domain not_subdomain("duck.ru"sv);
        assert(not_subdomain.IsSubdomain(domain) == false);
    }
    {
        Domain domain("com"sv);
        Domain subdomain("alter.duck.com"sv);
        assert(subdomain.IsSubdomain(domain));
    }
    {
        Domain domain("class.com"sv);
        Domain subdomain("class.com"sv);
        assert(subdomain.IsSubdomain(domain) == true);
        Domain not_subdomain("class.ru"sv);
        assert(not_subdomain.IsSubdomain(domain) == false);
    }
    {
        Domain domain("class.com"sv);
        Domain subdomain("duck.class.com"sv);
        assert(subdomain.IsSubdomain(domain));
        Domain not_subdomain("duck.com"sv);
        assert(not_subdomain.IsSubdomain(domain) == false);
    }
}

void TestReadDomains() {
    std::ostringstream str_out;
    // тестирование чтения не из пустого потока
    {
        const std::vector<Domain> domains = {"gdz.ru"sv,
                                             "gdz.com"sv,
                                             "m.maps.me"sv,
                                             "alg.m.gdz.ru"sv,
                                             "maps.com"sv,
                                             "maps.ru"sv,
                                             "gdz.ua"sv
        };
        str_out << domains;

        std::istringstream in_str(str_out.str());
    
        const std::vector<Domain> test_domains = ReadDomains(in_str, domains.size());
        assert(test_domains == domains);
    }
    // тестирование чтения из пустого потока
    {
        std::istringstream in_str;
        const std::vector<Domain> test_domains = ReadDomains(in_str, 0);
        assert(test_domains == std::vector<Domain>{});
    }
}

void TestDomainChecker() {
    std::ostringstream out_str;
    const std::vector<Domain> domains = {"gdz.ua"sv,
                                         "gdz.ub"sv,
                                         "gdz.uc"sv,
                                         "gdz.ud"sv,
                                         "gdz.uf"sv,
                                         "gdz.ug"sv
    };
    DomainChecker checker(domains.begin(), domains.end());
    out_str << checker;

    std::ostringstream out_str2;
    out_str2 << domains;

    std::string s1 = out_str.str();
    std::string s2 = out_str2.str();

    assert(out_str.str() == out_str2.str());
}

void TestIsForbidden() {
    const std::vector<Domain> test_domains = {"gdz.ru"sv,
                                              "gdz.com"sv,
                                              "m.maps.me"sv,
                                              "alg.m.gdz.ru"sv,
                                              "maps.com"sv,
                                              "maps.ru"sv,
                                              "gdz.ua"sv
    };
    // тестирование при непустом списке запрещённых доменов
    {
        std::ostringstream out_str;
        const std::vector<Domain> forbidden_domains = {"gdz.ru"sv,
                                                       "maps.me"sv,
                                                       "m.gdz.ru"sv,
                                                       "com"sv
        };
        DomainChecker checker(forbidden_domains.begin(), forbidden_domains.end());

        for (const Domain& domain : test_domains) {
            out_str << (checker.IsForbidden(domain) ? "Bad"sv : "Good"sv) << std::endl;
        }
        assert(out_str.str() == "Bad\nBad\nBad\nBad\nBad\nGood\nGood\n"sv);
    }
    // тестирование при пустом списке запрещённых доменов
    {
        std::ostringstream out_str;
        const std::vector<Domain> forbidden_domains = {};
        DomainChecker checker(forbidden_domains.begin(), forbidden_domains.end());

        for (const Domain& domain : test_domains) {
            out_str << (checker.IsForbidden(domain) ? "Bad"sv : "Good"sv) << std::endl;
        }
        assert(out_str.str() == "Good\nGood\nGood\nGood\nGood\nGood\nGood\n"sv);
    }
}

void Tests() {
    TestDomain();
    TestReadDomains();
    TestDomainChecker();
    TestIsForbidden();
}

int main() {
    const std::vector<Domain> forbidden_domains = ReadDomains(std::cin, ReadNumberOnLine<size_t>(std::cin));
    DomainChecker checker(forbidden_domains.begin(), forbidden_domains.end());

    const std::vector<Domain> test_domains = ReadDomains(std::cin, ReadNumberOnLine<size_t>(std::cin));
    for (const Domain& domain : test_domains) {
        std::cout << (checker.IsForbidden(domain) ? "Bad"sv : "Good"sv) << std::endl;
    }
    //Tests();
}