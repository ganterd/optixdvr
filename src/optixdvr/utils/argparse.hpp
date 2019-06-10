#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <typeinfo>

/**
 * Abstract Argument class. All arguments are based on this
 * abstract type
 */
class Argument
{
public:
	enum Type
	{
		Flag,
		String,
		Integer,
		Float,
		Switch,
		Generic
	};

	std::string shortSignature = "";
	std::string longSignature = "";
	std::string info = "";
	bool set = false;
	int requiredInputCount = 0;
	bool required = false;
	Type type;

	Argument(){};
	Argument(Type t) : type(t){};

	virtual void Parse(int argc, int& i, char* argv[]) = 0;
	virtual bool Validate() = 0;
	virtual std::string toString() = 0;
};

/**
 * Flag Argument class.
 *
 * Represents a present/nonpresent flag argument
 */
class FlagArgument : public Argument
{
public:
	FlagArgument() : Argument(Type::Flag){};
	virtual void Parse(int argc, int& i, char* argv[]){ set = true; };
	virtual bool Validate() { return true;  };
	virtual std::string toString(){ return set ? "true" : "false"; };
};

/**
 * String Argument class.
 *
 * Represents a string parameter argument. Will require
 * an additional argument after signature.
 */
class StringArgument : public Argument
{
public:
	std::string defaultValue = "";
	std::string value = "";

	StringArgument() : Argument(Type::String)
	{
		requiredInputCount = 1;
	};
	virtual void Parse(int argc, int& i, char* argv[]);
	virtual bool Validate();
	virtual std::string toString(){ return set ? value : defaultValue; }
};

/**
 * Integer Argument class.
 *
 * Represents an integer parameter argument. Will require
 * and additional argument after signature.
 */
class IntegerArgument : public Argument
{
public:
	int defaultValue = 0;
	int value = 0;

	IntegerArgument() : Argument(Type::Integer)
	{
		requiredInputCount = 1;
	};
	virtual void Parse(int argc, int& i, char* argv[]);
	virtual bool Validate(){ return true; };
	virtual std::string toString(){ return std::to_string(set ? defaultValue : value); };
};

/**
 * Float Argument class.
 *
 * Represents an integer parameter argument. Will require
 * and additional argument after signature.
 */
class FloatArgument : public Argument
{
public:
	float defaultValue = 0.0f;
	float value = 0.0f;

	FloatArgument() : Argument(Type::Float)
	{
		requiredInputCount = 1;
	};
	virtual void Parse(int argc, int& i, char* argv[]);
	virtual bool Validate(){ return true; };
	virtual std::string toString(){ return std::to_string(set ? defaultValue : value); };
};

template <typename T> class GenericArgument : public Argument
{
public:
	T defaultValue;
	T value;

	GenericArgument() : Argument(Type::Generic)
	{
		requiredInputCount = 1;
	}
	virtual void Parse(int argc, int& i, char* argv[])
	{	
		if(i + 1 >= argc)
		{
			std::cerr << argv[i] << " requires one argument." << std::endl;
			return;
		}

		set = true;

		std::stringstream ss(argv[++i]);
		ss >> value;
	}

	virtual bool Validate(){ return true; };
	virtual std::string toString(){ return std::to_string(set ? defaultValue : value); };
};

/**
 * Switch Argument class.
 *
 * Represents a switch argument. Will require and
 * additional argument after signature and this value
 * must be one of the specified switch options.
 */
class SwitchArgument : public Argument
{
public:
	std::vector<std::string> options;
	std::string defaultValue = "";
	std::string value = "";

	SwitchArgument() : Argument(Type::Switch)
	{
		requiredInputCount = 1;
	};
	virtual void Parse(int argc, int& i, char* argv[]);
	virtual bool Validate();
	virtual std::string toString(){ return set ? value : defaultValue; };
};

/**
 * Arguments class.
 *
 * Is effectively the manager of all defined arguments and
 * does the bulk of the argument passing work.
 */
class Arguments
{
private:
	typedef std::map<std::string, Argument*> ParameterMap;
	static ParameterMap parameters;
	static ParameterMap::iterator FindParameter(std::string arg)
	{
		for(ParameterMap::iterator it = parameters.begin(); it != parameters.end(); ++it)
		{
			Argument* p = it->second;
			if(arg == p->shortSignature || arg == p->longSignature)
				return it;
		}
		return parameters.end();
	}

public:
	/**
	 * Adds an argument to be available for use.
	 * 
	 * @param name Name used to look up the status of the argument
	 * @param arg Allocated and instantiated argument.
	 */
	static void AddAvailableArgument(std::string name, Argument* arg);

	/**
	 * Add a flag type argument to the available argument list.
	 * 
	 * @param name        Name used for status look-up.
	 * @param shortArgSig Short signature used for argument.
	 * @param longArgSig  Long signature used for argument.
	 */
	static void AddFlagArgument(
		std::string name,
		std::string shortArgSig, 
		std::string longArgSig
	);

	/**
	 * Adds a string type argument to the available argument list.
	 * 
	 * @param name        Name used for status look-up.
	 * @param shortArgSig Short signature used for argument.
	 * @param longArgSig  Long signature used for argument.
	 * @param defaultVal  Default value of the string argument.
	 */
	static void AddStringArgument(
		std::string name,
		std::string shortArgSig, 
		std::string longArgSig,
		std::string defaultVal
	);

	/**
	 * Adds an integer type argument to the available argument list.
	 * 
	 * @param name       Name used for status look-up.
	 * @param shortSig   Short signature used for argument.
	 * @param longSig    Long signature used for argument.
	 * @param defaultVal Default value of the integer argument.
	 */
	static void AddIntegerArgument(
		std::string name,
		std::string shortSig,
		std::string longSig,
		int defaultVal
	);

    /**
	 * Adds an integer type argument to the available argument list.
	 * 
	 * @param name       Name used for status look-up.
	 * @param shortSig   Short signature used for argument.
	 * @param longSig    Long signature used for argument.
	 * @param defaultVal Default value of the integer argument.
	 */
	static void AddFloatArgument(
		std::string name,
		std::string shortSig,
		std::string longSig,
		float defaultVal
	);

	/**
	 * Adds a switch type argument to the available argument list
	 * with defined options.
	 * 
	 * @param name        Name used for status look-up.
	 * @param shortArgSig Short signature used for argument.
	 * @param longArgSig  Long signature used for argument.
	 * @param options     List of options allowed for switch.
	 * @param defaultVal  Default switch value. Must be one of options.
	 */
	static void AddSwitchArgument(
		std::string name,
		std::string shortArgSig,
		std::string longArgSig,
		std::vector<std::string> options,
		std::string defaultVal
	);

	template <typename T> static void AddArgument(
		std::string name,
		std::string shortArgSig,
		std::string longArgSig,
		const T& defaultVal
	)
	{
		GenericArgument<T>* arg = new GenericArgument<T>();
		arg->shortSignature = shortArgSig;
		arg->longSignature = longArgSig;
		arg->defaultValue = defaultVal;
		Arguments::AddAvailableArgument(name, arg);
	}

	/**
	 * Sets the info for an argument. Appears if something goes
	 * wrong when parsing the argument input or fails validations.
	 * 
	 * @param name Argument to set info for.
	 * @param info String with argument info.
	 */
	static void SetArgumentInfo(std::string name, std::string info);

	/**
	 * Defined if an argument is required. If a value is not set and
	 * Arguments::Validate() is called, the method will return false.
	 * 
	 * @param arg      Argument to set required status.
	 * @param required Boolean required value.
	 */
	static void SetArgumentRequired(std::string arg, bool required);

	/**
	 * Parse a list of arguments passed on the commandline.
	 * 
	 * @param argc Argument count.
	 * @param argv Array of chars storing arguments.
	 */
	static void Parse(int argc, char* argv[]);

	/**
	 * Validates the current argument states.
	 * 
	 * @return Will return false if an argument is required and is not set.
	 */
	static bool Validate();

	/**
	 * Function to check if an argument had been definied.
	 * 
	 * @param  arg Name of argument to get if defined.
	 * @return     True if argument is defined.
	 */
	static bool IsArgument(std::string arg);

	/**
	 * Function to check if an argument has been set.
	 * @param  arg Name of argument to check if set.
	 * @return     True if set, false if not or if argument not defined.
	 */
	static bool IsSet(std::string arg);

	/**
	 * Function to get value of a StringArgument.
	 * 
	 * @param  arg Name of argument to get value of.
	 * @return     String value of argument.
	 */
	static std::string GetAsString(std::string arg);

	/**
	 * Function to get value of an IntegerArgument.
	 * 
	 * @param  arg Name of argument to get value of.
	 * @return     Int value of argument.
	 */
	static int GetAsInt(std::string arg);

    /**
	 * Function to get value of an IntegerArgument.
	 * 
	 * @param  arg Name of argument to get value of.
	 * @return     Int value of argument.
	 */
	static float GetAsFloat(std::string arg);

	/**
	 * Function to get value of an IntegerArgument, represented as
	 * an unsigned int.
	 * 
	 * @param  arg Name of argument to get value of.
	 * @return     UInt value of argument.
	 */
	static unsigned int GetAsUInt(std::string arg);

	template <typename T> static T Get(const std::string& arg)
	{
		if(!IsArgument(arg))
			std::cerr  << "No argument with name '" << arg << "' available to get!" << std::endl;

		GenericArgument<T>* a = dynamic_cast<GenericArgument<T>*>(parameters[arg]);
		if(a == NULL)
			std::cerr << "Argument '" << arg << "' us not the same type as requested (" << typeid(T).name() << ")" << std::endl;

		return a->set ? a->value : a->defaultValue;
	};
};