#include "argparse.hpp"

Arguments::ParameterMap Arguments::parameters;

void Arguments::AddAvailableArgument(std::string name, Argument* arg)
{
	Arguments::parameters[name] = arg;
}

void Arguments::AddFlagArgument(
	std::string name,
	std::string shortArgSig,
	std::string longArgSig
)
{
	FlagArgument* arg = new FlagArgument();
	arg->shortSignature = shortArgSig;
	arg->longSignature = longArgSig;
	Arguments::AddAvailableArgument(name, arg);
}

void Arguments::AddStringArgument(
	std::string name,
	std::string shortArgSig,
	std::string longArgSig,
	std::string defaultVal
)
{
	StringArgument* arg = new StringArgument();
	arg->shortSignature = shortArgSig;
	arg->longSignature = longArgSig;
	arg->defaultValue = defaultVal;
	Arguments::AddAvailableArgument(name, arg);
}

void Arguments::AddIntegerArgument(
	std::string name,
	std::string shortSig,
	std::string longSig,
	int defaultVal
)
{
	IntegerArgument* arg = new IntegerArgument();
	arg->shortSignature = shortSig;
	arg->longSignature = longSig;
	arg->defaultValue = defaultVal;
	Arguments::AddAvailableArgument(name, arg);
}

void Arguments::AddFloatArgument(
	std::string name,
	std::string shortSig,
	std::string longSig,
	float defaultVal
)
{
	FloatArgument* arg = new FloatArgument();
	arg->shortSignature = shortSig;
	arg->longSignature = longSig;
	arg->defaultValue = defaultVal;
	Arguments::AddAvailableArgument(name, arg);
}

void Arguments::AddSwitchArgument(
	std::string name,
	std::string shortArgSig,
	std::string longArgSig,
	std::vector<std::string> options,
	std::string defaultVal
)
{
	SwitchArgument* arg = new SwitchArgument();
	arg->shortSignature = shortArgSig;
	arg->longSignature = longArgSig;
	arg->options = options;
	arg->defaultValue = defaultVal;
	arg->value = defaultVal;
	arg->required = false;
	Arguments::AddAvailableArgument(name, arg);
}

void Arguments::SetArgumentInfo(std::string name, std::string info)
{
	if(IsArgument(name))
	{
		parameters[name]->info = info;
	}
}

void Arguments::SetArgumentRequired(std::string name, bool required)
{
	if(!IsArgument(name))
	{
		std::cerr << name << " is not a listed available argument." << std::endl;
		return;
	}

	parameters[name]->required = required;
}

void StringArgument::Parse(int argc, int& i, char* argv[])
{
	if(i + 1 >= argc)
	{
		std::cerr << argv[i] << " requires one string argument." << std::endl;
		return;
	}

	set = true;
	value = std::string(argv[++i]);
}

bool StringArgument::Validate()
{
	if(required)
		if(set)
			return true;
		else
		{
			std::cerr << "No value set for required argument '" << shortSignature << "'";
			if(info.size() > 0)
				std::cerr << " - " << info;
			std::cerr << std::endl;
			return false;
		}
	return true;
}

void IntegerArgument::Parse(int argc, int& i, char* argv[])
{
	if(i + 1 >= argc)
	{
		std::cerr << argv[i] << " required one integer argument." << std::endl;
		return;
	}

	set = true;
	value = std::stoi(argv[++i]);
}

void FloatArgument::Parse(int argc, int& i, char* argv[])
{
	if(i + 1 >= argc)
	{
		std::cerr << argv[i] << " required one float argument." << std::endl;
		return;
	}

	set = true;
	value = std::stof(argv[++i]);
}

void SwitchArgument::Parse(int argc, int& i, char* argv[])
{
	if(i + 1 < argc)
	{
		value = std::string(argv[++i]);
		set = true;
		return;
	}
}

bool SwitchArgument::Validate()
{
	for(int v = 0; v < options.size(); ++v)
	{
		if(options[v] == value)
			return true;
	}

	std::cerr << shortSignature << " requires one of the following options: [";
	for(int v = 0; v < options.size(); ++v)
	{
		if(v > 0)
			std::cerr << "|";
		std::cerr << options[v];
	}
	std::cerr << "]" << std::endl;

	return false;
}

void Arguments::Parse(int argc, char* argv[])
{
	for (int i = 1; i < argc; ++i)
	{
		ParameterMap::iterator it = FindParameter(std::string(argv[i]));
		if (it != parameters.end())
		{

			Argument* p = it->second;
			p->Parse(argc, i, argv);
		}
		else
		{
			std::cerr << "Unrecognised parameter \"" << argv[i] << "\"" << std::endl;
		}
	}
}

bool Arguments::Validate()
{
	for(ParameterMap::iterator it = parameters.begin(); it != parameters.end(); ++it)
	{
		Argument* p = it->second;
		if(!p->Validate())
			return false;
	}

	return true;
}

bool Arguments::IsArgument(std::string arg)
{
	ParameterMap::iterator it = FindParameter(arg);
	if(it != parameters.end())
		return false;
	return true;
}

bool Arguments::IsSet(std::string arg)
{
	if(!IsArgument(arg))
		return false;
	return parameters[arg]->set;
}

std::string Arguments::GetAsString(std::string name)
{
	if(!Arguments::IsArgument(name))
	{
		std::cerr << "No argument of name '" << name << "'" << std::endl;
		return "";
	}

	return parameters[name]->toString();
}

int Arguments::GetAsInt(std::string name)
{
	if(!Arguments::IsArgument(name))
	{
		std::cerr << "No argument of name'" << name << "'" << std::endl;
		return 0;
	}

	if(parameters[name]->type != Argument::Type::Integer)
	{
		std::cerr << "Arg '" << name << "' is not an integer type." << std::endl;
		return 0;
	}

	IntegerArgument* iarg = (IntegerArgument*)parameters[name];

	if(!iarg->set)
		return iarg->defaultValue;
	return iarg->value;
}

float Arguments::GetAsFloat(std::string name)
{
	if(!Arguments::IsArgument(name))
	{
		std::cerr << "No argument of name'" << name << "'" << std::endl;
		return 0;
	}

	if(parameters[name]->type != Argument::Type::Float)
	{
		std::cerr << "Arg '" << name << "' is not a float type." << std::endl;
		return 0;
	}

	FloatArgument* iarg = (FloatArgument*)parameters[name];

	if(!iarg->set)
		return iarg->defaultValue;
	return iarg->value;
}

unsigned int Arguments::GetAsUInt(std::string name)
{

	return (unsigned int)GetAsInt(name);
}