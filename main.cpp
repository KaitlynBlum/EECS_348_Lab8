
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

// A parsed number is still stored entirely as strings.
// The sign is stored separately so arithmetic is easier to manage.
struct DecimalNumber {
    bool negative;
    string integerPart;
    string fractionalPart;
};

// Remove leading zeros from an integer string.
// Keep one zero if the entire string consists of zeros.
string removeLeadingZeros(const string& s) {
    size_t first = s.find_first_not_of('0');

    if (first == string::npos) {
        return "0";
    }

    return s.substr(first);
}

// Return true only if s follows the required number format:
//
//     (+|-)A(.B)
//
// A must contain at least one digit.
// If a decimal point is present, B must also contain at least one digit.
// Only an optional sign at the beginning is allowed.
bool isValidNumber(const string& s) {
    if (s.empty()) {
        return false;
    }

    size_t i = 0;

    // An optional sign is allowed at the beginning.
    if (s[i] == '+' || s[i] == '-') {
        ++i;
    }

    // A digit must follow the optional sign.
    if (i >= s.size()) {
        return false;
    }

    bool foundDecimalPoint = false;
    bool foundDigitBeforeDecimal = false;
    bool foundDigitAfterDecimal = false;

    // Examine each character manually. No numeric conversion is used.
    for (; i < s.size(); ++i) {
        char c = s[i];

        if (c >= '0' && c <= '9') {
            if (foundDecimalPoint) {
                foundDigitAfterDecimal = true;
            } else {
                foundDigitBeforeDecimal = true;
            }
        } else if (c == '.') {
            // A second decimal point makes the number invalid.
            if (foundDecimalPoint) {
                return false;
            }

            foundDecimalPoint = true;
        } else {
            // Reject letters, spaces inside a token, and all other
            // characters, including additional signs and exponent notation.
            return false;
        }
    }

    if (!foundDigitBeforeDecimal) {
        return false;
    }

    // If there was a decimal point, it must have digits on both sides.
    if (foundDecimalPoint && !foundDigitAfterDecimal) {
        return false;
    }

    return true;
}

// Convert a valid number string into its sign, integer part,
// and fractional part. This only splits strings; it does not
// convert the complete number into a built-in numeric type.
DecimalNumber parseNumber(const string& s) {
    DecimalNumber number;
    number.negative = false;

    size_t start = 0;

    if (s[0] == '-') {
        number.negative = true;
        start = 1;
    } else if (s[0] == '+') {
        start = 1;
    }

    // Find the decimal point, if one exists.
    size_t dot = s.find('.', start);

    if (dot == string::npos) {
        number.integerPart = s.substr(start);
        number.fractionalPart = "";
    } else {
        number.integerPart = s.substr(start, dot - start);
        number.fractionalPart = s.substr(dot + 1);
    }

    // Leading integer zeros do not affect the value.
    number.integerPart = removeLeadingZeros(number.integerPart);

    // Trailing fractional zeros do not affect the value either.
    while (!number.fractionalPart.empty() &&
           number.fractionalPart.back() == '0') {
        number.fractionalPart.pop_back();
    }

    // A value with no nonzero digits is zero, regardless of its sign.
    bool allZero = true;

    for (char c : number.integerPart) {
        if (c != '0') {
            allZero = false;
            break;
        }
    }

    for (char c : number.fractionalPart) {
        if (c != '0') {
            allZero = false;
            break;
        }
    }

    if (allZero) {
        number.negative = false;
        number.integerPart = "0";
        number.fractionalPart = "";
    }

    return number;
}

// Compare two nonnegative integer strings.
// Return -1 if a < b, 0 if a == b, and 1 if a > b.
// The strings should not have unnecessary leading zeros.
int compareMagnitude(const string& a, const string& b) {
    if (a.size() < b.size()) {
        return -1;
    }

    if (a.size() > b.size()) {
        return 1;
    }

    if (a < b) {
        return -1;
    }

    if (a > b) {
        return 1;
    }

    return 0;
}

// Add two nonnegative integer strings using digit-by-digit addition.
// Carry is an integer for an individual digit, not an entire number.
string addMagnitude(const string& a, const string& b) {
    string result;

    int i = static_cast<int>(a.size()) - 1;
    int j = static_cast<int>(b.size()) - 1;
    int carry = 0;

    while (i >= 0 || j >= 0 || carry > 0) {
        int digitA = 0;
        int digitB = 0;

        if (i >= 0) {
            digitA = a[i] - '0';
            --i;
        }

        if (j >= 0) {
            digitB = b[j] - '0';
            --j;
        }

        int sum = digitA + digitB + carry;

        result.push_back(static_cast<char>('0' + (sum % 10)));
        carry = sum / 10;
    }

    // Digits were generated from right to left.
    reverse(result.begin(), result.end());

    return removeLeadingZeros(result);
}

// Subtract b from a, where a >= b and both are nonnegative
// integer strings. Borrow is handled one digit at a time.
string subtractMagnitude(const string& a, const string& b) {
    string result;

    int i = static_cast<int>(a.size()) - 1;
    int j = static_cast<int>(b.size()) - 1;
    int borrow = 0;

    while (i >= 0) {
        int digitA = (a[i] - '0') - borrow;
        int digitB = 0;

        if (j >= 0) {
            digitB = b[j] - '0';
        }

        if (digitA < digitB) {
            digitA += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }

        int difference = digitA - digitB;
        result.push_back(static_cast<char>('0' + difference));

        --i;
        --j;
    }

    reverse(result.begin(), result.end());

    return removeLeadingZeros(result);
}

// Remove unnecessary leading zeros from the integer portion and
// trailing zeros from the fractional portion of a formatted result.
string formatResult(string digits, size_t decimalPlaces, bool negative) {
    // Ensure there are enough digits to leave at least one integer digit.
    while (digits.size() <= decimalPlaces) {
        digits.insert(digits.begin(), '0');
    }

    string integerPart;
    string fractionalPart;

    if (decimalPlaces == 0) {
        integerPart = digits;
        fractionalPart = "";
    } else {
        size_t splitPosition = digits.size() - decimalPlaces;
        integerPart = digits.substr(0, splitPosition);
        fractionalPart = digits.substr(splitPosition);
    }

    integerPart = removeLeadingZeros(integerPart);

    // Decimal trailing zeros are not needed in the displayed result.
    while (!fractionalPart.empty() && fractionalPart.back() == '0') {
        fractionalPart.pop_back();
    }

    // Detect zero after formatting so negative zero is never printed.
    bool isZero = (integerPart == "0");

    for (char c : fractionalPart) {
        if (c != '0') {
            isZero = false;
            break;
        }
    }

    string result;

    if (negative && !isZero) {
        result += '-';
    }

    result += integerPart;

    if (!fractionalPart.empty()) {
        result += '.';
        result += fractionalPart;
    }

    return result;
}

// Add two valid decimal strings without converting either operand
// to double, float, long, long long, or another whole-number type.
string addDecimals(const string& first, const string& second) {
    DecimalNumber a = parseNumber(first);
    DecimalNumber b = parseNumber(second);

    // Align the fractional portions by appending zeros to the shorter
    // fractional string. For example, 1.2 becomes 1.200 if needed.
    size_t scale = max(a.fractionalPart.size(),
                       b.fractionalPart.size());

    a.fractionalPart.append(scale - a.fractionalPart.size(), '0');
    b.fractionalPart.append(scale - b.fractionalPart.size(), '0');

    // Join each integer and fractional portion into one digit string.
    // These strings represent scaled integers, not built-in numbers.
    string digitsA = a.integerPart + a.fractionalPart;
    string digitsB = b.integerPart + b.fractionalPart;

    digitsA = removeLeadingZeros(digitsA);
    digitsB = removeLeadingZeros(digitsB);

    string resultDigits;
    bool resultNegative = false;

    if (a.negative == b.negative) {
        // Same signs: add magnitudes and keep their common sign.
        resultDigits = addMagnitude(digitsA, digitsB);
        resultNegative = a.negative;
    } else {
        // Opposite signs: subtract the smaller magnitude from
        // the larger magnitude. The larger one's sign wins.
        int comparison = compareMagnitude(digitsA, digitsB);

        if (comparison == 0) {
            // Equal magnitudes with opposite signs produce zero.
            resultDigits = "0";
            resultNegative = false;
        } else if (comparison > 0) {
            resultDigits = subtractMagnitude(digitsA, digitsB);
            resultNegative = a.negative;
        } else {
            resultDigits = subtractMagnitude(digitsB, digitsA);
            resultNegative = b.negative;
        }
    }

    // Restore the decimal point and remove unnecessary zeros.
    return formatResult(resultDigits, scale, resultNegative);
}

// Print invalid tokens in the format used by the supplied expected output.
// If both tokens are invalid, show both in the same error message.
void printInvalidNumbers(const string& first, const string& second) {
    bool firstValid = isValidNumber(first);
    bool secondValid = isValidNumber(second);

    cout << "ERROR: invalid number";

    if (!firstValid) {
        cout << " \"" << first << "\"";
    }

    if (!secondValid) {
        cout << " \"" << second << "\"";
    }

    cout << '\n';
}

// Process the input file line by line. Every physical input line
// receives its own line number, including malformed lines.
void processFile(const string& filename) {
    ifstream inputFile(filename);

    if (!inputFile.is_open()) {
        cerr << "ERROR: could not open input file \"" << filename
             << "\"\n";
        return;
    }

    string line;
    size_t lineNumber = 0;

    while (getline(inputFile, line)) {
        ++lineNumber;

        // Tokenize the current line using whitespace separation.
        istringstream lineStream(line);
        vector<string> tokens;
        string token;

        while (lineStream >> token) {
            tokens.push_back(token);
        }

        // The specification requires exactly two values per line.
        if (tokens.size() != 2) {
            cout << "line " << lineNumber
                 << ": ERROR: expected exactly two numbers\n";
            continue;
        }

        const string& first = tokens[0];
        const string& second = tokens[1];

        bool firstValid = isValidNumber(first);
        bool secondValid = isValidNumber(second);

        // Invalid values are reported, and processing continues.
        if (!firstValid || !secondValid) {
            cout << "line " << lineNumber << ": ";
            printInvalidNumbers(first, second);
            continue;
        }

        // All arithmetic is performed using strings.
        string result = addDecimals(first, second);

        cout << "line " << lineNumber << ": "
             << first << " + " << second << " = " << result << '\n';
    }

    inputFile.close();
}

int main(int argc, char* argv[]) {
    // The filename may be provided on the command line.
    // If none is provided, use tests.txt.
    // This avoids hard-coding a filename that cannot be changed.
    string filename = "tests.txt";

    if (argc >= 2) {
        filename = argv[1];
    }

    processFile(filename);

    return 0;
}
