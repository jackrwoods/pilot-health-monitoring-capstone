#include <vector>
#include <ctime>
#include <functional>

#include <wiringPi.h>

/**
 * Variable_Mapping: Store the pin number and the variable that it controls.
 */
struct Variable_Mapping
{
    int &value;
    int wiringpi_pin;

    Variable_Mapping(int &v, int pi_pin);
};

Variable_Mapping::Variable_Mapping(int &v, int pi_pin) : value{v}, wiringpi_pin{pi_pin}
{
}

/**
 * Function_Mapping: Store the pin number and the function that input from the pin should call. 
 * Implemented with a cool off to allow the function to be called only once every input.
 */
struct Function_Mapping
{
    // void (*function)();
    std::function<void()> function;
    int wiringpi_pin;
    time_t cool_off{0};
    time_t last_call{0};

    Function_Mapping(void (*f)(), int pi_pin, time_t cooloff_seconds);
    Function_Mapping(std::function<void()> f, int pi_pin, time_t cooloff_seconds);
};

/**
 * Function_Mapping: Constructor for creating a Function_Mapping object. This is used to map functions to input pins on the Raspberry Pi.
 * @param f The function to be called when input is detected on the pin.
 * @param pi_pin The input pin to map the function to (uses WiringPi pin numberings).
 * @param cooloff_seconds Minimum number of seconds in between function calls.
 */
Function_Mapping::Function_Mapping(void (*f)(), int pi_pin, time_t cooloff_seconds) : function{f}, wiringpi_pin{pi_pin}, cool_off{cooloff_seconds}
{
}

/**
 * Function_Mapping: Constructor for creating a Function_Mapping object. This is used to map functions to input pins on the Raspberry Pi.
 * @param f The function to be called when input is detected on the pin.
 * @param pi_pin The input pin to map the function to (uses WiringPi pin numberings).
 * @param cooloff_seconds Minimum number of seconds in between function calls.
 */
Function_Mapping::Function_Mapping(std::function<void()> f, int pi_pin, time_t cooloff_seconds) : function{f}, wiringpi_pin{pi_pin}, cool_off{cooloff_seconds}
{
}

/**
 * Button_Input: Wrapper around WiringPi libary that simplifies changing variables and calling functions on Raspberry Pi pin input.
 */
class Button_Input
{
private:
    std::vector<Variable_Mapping> variables;
    std::vector<Function_Mapping> functions;

    bool is_quit{false};

public:
    Button_Input();

    void add_variable_mapping(Variable_Mapping &&b);
    void add_function_mapping(Function_Mapping &&b);

    void quit();

    void run();
};

Button_Input::Button_Input()
{
    wiringPiSetup();
}

/**
 * add_variable_mapping: Add a variable to be changed when input is detected on a pin.
 * @param b Variable_Mapping struct created using the Variable_Mapping(int &, int) constructor.
 */
void Button_Input::add_variable_mapping(Variable_Mapping &&b)
{
    variables.push_back(b);
    pinMode(b.wiringpi_pin, INPUT);
}

/**
 * add_function_mapping: Add a function to be called when input is detected on a pin.
 * @param b Function_Mapping struct created using a Function_Mapping constructor.
 */
void Button_Input::add_function_mapping(Function_Mapping &&b)
{
    functions.push_back(b);
    pinMode(b.wiringpi_pin, INPUT);
}

/**
 * quit: Stop the execution of the run() function.
 */
void Button_Input::quit()
{
    is_quit = true;
}

/**
 * run: Run a loop that reads pins and updates variables/calls functions. Runs until quit() is called.
 */
void Button_Input::run()
{
    while (is_quit == false)
    {
        // variables
        for (auto i = variables.begin(); i != variables.end(); i++)
        {
            if (digitalRead(i->wiringpi_pin))
                i->value = 0;
            else
                i->value = 1;
        }

        // functions
        for (auto i = functions.begin(); i != functions.end(); i++)
        {
            if (!digitalRead(i->wiringpi_pin) && time(0) - i->last_call >= i->cool_off)
            {
                i->function();
                i->last_call = time(0);
            }
        }
    }
}