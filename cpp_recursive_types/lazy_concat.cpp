/**
 * * From the discussion in
 * << Ivan Cukic's Functional Programming in C++ [Section 6.4] >>
 *
 * Use template parameter packs and Haskell-like pattern matching
 * to create recursive types implementing natural constructs with
 * no overhead.
 *
 * Here, we implement a data structure reminiscent of a Lisp list
 * (without the indirection!) representing the concatenation of
 * an arbitrary amount of std::string objects. As a result of the abstraction,
 * concatenation is performed lazily and the memory buffer for storing the
 * end result is only allocated if/when the object is (projected down to) an
 * actual std::string object.
 */
#include <algorithm>
#include <iostream>
#include <string>


/**
 * Meta type implementing lazy concatenation.
 */
template <typename... Tail> class lazy_concatenator;






/**
 * General case of the recursion
 */
template <typename Head, typename... Tail>
class lazy_concatenator<Head, Tail...>
{
private:
  /* Data of the original string */
  Head m_head;
  /* Structure containing the rest of the pieces */
  lazy_concatenator<Tail...> m_tail;

public:
  /**
   * In the spirit of Haskell pattern matching, store the head of the data
   * along with some object representing the remainder of the concatenated strings.
   */
  lazy_concatenator(Head data, lazy_concatenator<Tail...> tail)
      : m_head(data), m_tail(tail) {}

  /**
   * Can compute the size of the amassed data recursively
   */
  int size() const { return m_head.size() + m_tail.size(); }

  /**
   * Copy the content recursively to some given output iterator
   */
  template <typename OutputIterator> void save(OutputIterator end) const {
    auto begin = end - m_head.size();
    std::copy(m_head.begin(), m_head.end(), begin);
    m_tail.save(begin);
  }

  /**
   * Concatenation operation uses 'polymorphic recursion'
   */
  lazy_concatenator<std::string, Head, Tail...>
  operator+(const std::string &other) const {
    return lazy_concatenator<std::string, Head, Tail...>(other, *this);
  }

  /**
   * Implicit conversion operator to an actual std::string
   */
  operator std::string() const {
    std::string result(size(), '\0');
    save(result.end());
    return result;
  }

  /**
   * With an operator= implemented, we can pause and resume concatenation as we want
   */
  lazy_concatenator<Head, Tail...> &operator=(const lazy_concatenator &other) {
    m_head = other.m_head;
    m_tail = other.tail;
  }
};





/**
 * Base case of the recursion
 */
template <> class lazy_concatenator<> {
public:
  lazy_concatenator()
  {
  }

  int size() const
  {
    return 0;
  }

  template <typename OutputIter>
  void save(OutputIter) const
  {
  }

  lazy_concatenator<std::string>
  operator+(const std::string &other)
  {
    return lazy_concatenator<std::string>(other, *this);
  }
};





int main(int argc, char *argv[]) {
  using lazy_concat = lazy_concatenator<>;

  std::string hello = "Hello";
  std::string to = "to";
  std::string the = "the";
  std::string world = "world";


  lazy_concat LAZY;

  /// Each time we append a new string, we get a new type.
  /// Each one of those types encodes the way the building strings should be concatenated
  auto quantum_string = LAZY + hello + " " + to + " " + the + " " + world;

  static_assert(not std::is_same_v<
                decltype(LAZY + hello + to),
                decltype(quantum_string)
                >);

  static_assert(not std::is_same_v<
                decltype(quantum_string),
                std::string
                >);


  std::cout << "\n\n\n"
            << "LAZY + \"hello + \" \" + to + \" \" + the \" \" + world \""
            << "is not a string, but its size can be computed as "
            << quantum_string.size() << std::endl;

  // Compared to the naive approach of constructing (and allocating memory for) a new string object
  // each time the associative, binary operator `+` is used,
  // The memory allocation only happens once we convert the `quantum string` to a std::string object here
  std::string concatenated = quantum_string;
  std::cout << "The std::string object has now been constructed. It's content is\n  "
    << concatenated << "\n which is of size " << concatenated.size() << std::endl;

  // With our basic operator=, we can resume concatenation if we want to...
  auto quantum_string_continued =
    quantum_string + " Can resume " + "concatenating " + "more " +
    "strings " + "too...";

  std::string concatenated_more = quantum_string_continued;
  std::cout << "\n\n\n\nWe can still add more stuff:\n  " << concatenated_more
            << std::endl;

  // NOTE: We could even completely forgo any copying of the strings until this final one by implementing the
  // `lazy_concatenator::operator+` in a way that simply passes a pointer to the underlying buffer of the
  // component std::string (e.g. using move semantics...)
  // or by using shared pointers and reference counts in case of a larger scale structure based on these ideas...

  return 0;
}
