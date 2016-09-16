#ifndef __MYLIB_CONFIG_H__
#define __MYLIB_CONFIG_H__

#include <map>
#include <set>
#include <string>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cassert>
#include <vector>
#include <algorithm>
#include <iterator>
#include <iomanip>
#include <iostream>
#include "unistd.h"

class ConfigError : public std::runtime_error {
public:
	ConfigError(const std::string &msg) : runtime_error(msg) {;}
};

class ConfigBase {
public:
	ConfigBase(void)
		: _optstring(":") /// to distinguish the missing argument error and 
						  /// the invalid option error
	{}

	virtual ~ConfigBase(void) {}

	void write(std::ostream &ost, const std::string &header = "") const
	{
		for (size_t i = 0; i < _order.size(); i++) {
			std::string name = _order[i].first;
			Type type = _order[i].second;
			switch (type) {
				case STRING:
					writeValue(ost, header, _sValue, name);
					break;
				case INT:
					writeValue(ost, header, _iValue, name);
					break;
				case DOUBLE:
					writeValue(ost, header, _dValue, name);
					break;
				case BOOL:
					writeValue(ost, header, _bValueStr, name);
					break;
				case STRING_LIST:
					writeValue(ost, header, _slValue, name);
					break;
			}
		}
	}

	void help(std::ostream &ost) const
	{
		std::map<std::string, char> invoptchar;
		for (std::map<char, std::string>::const_iterator i = _optchar.begin();
				i != _optchar.end(); ++i) {
			invoptchar.insert(std::make_pair(i->second, i->first));
		}

		ost << "Options: (command-line-option, config-value-name, description)"
			<< std::endl;
		for (size_t i = 0; i < _order.size(); i++) {

			std::string name = _order[i].first;
			Type type = _order[i].second;

			if (isHidden(name)) {
				continue;
			}

			std::map<std::string, char>::const_iterator optItr
				= invoptchar.find(name);
			char opt = (optItr != invoptchar.end()) ? optItr->second : 0;

			std::map<std::string, std::string>::const_iterator descItr
				= _desc.find(name);
			std::string desc;
			if (descItr != _desc.end()) {
				desc = descItr->second;
			}

			switch (type) {
				case STRING:
					printHelpLine(
						ost, name, opt, *_sValue.find(name)->second, desc);
					break;
				case INT:
					printHelpLine(
						ost, name, opt, *_iValue.find(name)->second, desc);
					break;
				case DOUBLE:
					printHelpLine(
						ost, name, opt, *_dValue.find(name)->second, desc);
					break;
				case BOOL:
					printHelpLine(
						ost, name, opt, _bValueStr.find(name)->second, desc);
					break;
				case STRING_LIST:
					printHelpLine(
						ost, name, opt, *_slValue.find(name)->second, desc);
					break;
			}
		}

		std::exit(0);
	}

private:
	enum Type { STRING, INT, DOUBLE, BOOL, STRING_LIST };

	template<class DataType, Type typeCode>
	void defineImpl(
		std::map<std::string, DataType*> &valueMap,
 		const std::string &name,
 		DataType *value,
 		char opt,
		const std::string &desc
	) {
		_order.push_back(make_pair(name, typeCode));
		setType(name, typeCode);
		valueMap[name] = value;
		if (opt) {
			setOptChar(name, opt);
		}
		_desc[name] = desc;
	}

protected:

	void define(const std::string &name, std::string *value, char opt = 0)
	{
		defineImpl<std::string, STRING>(_sValue, name, value, opt, "");
	}

	void define(const std::string &name, int *value, char opt = 0)
	{
		defineImpl<int, INT>(_iValue, name, value, opt, "");
	}

	void define(const std::string &name, double *value, char opt = 0)
	{
		defineImpl<double, DOUBLE>(_dValue, name, value, opt, "");
	}

	void define(const std::string &name, bool *value, char opt = 0)
	{
		defineImpl<bool, BOOL>(_bValue, name, value, opt, "");
		_bValueStr[name] = (*value) ? "yes" : "no";
	}

	void define(const std::string &name, std::vector<std::string> *value)
	{
		//// Do not allow command-line option
		defineImpl<std::vector<std::string>, STRING_LIST>(
			_slValue, name, value, 0, "");
	}

	void readConfFile(const std::string &fname)
	{
		std::ifstream f(fname.c_str());
		if (! f) {
			throw ConfigError("cannot open config file \"" + fname + "\"");
		}

		std::string line;
		while (std::getline(f, line)) {
			if (line.length() == 0 || line[0] == '#') {
				continue;
			}
			std::string name;
			std::string svalue;
			std::istringstream iss(line.c_str());
			if (! (iss >> name >> svalue)) {
				throw ConfigError(
					"configration file format error\n"
					"line : " + line);
			}

			//// Skip assignment if already assigned in command-line option
			if (isDefined(name)) {
				continue;
			}

			setValue(name, svalue);
		}
	}

	/// TODO: detect the case where an argument is specified for an option
	///       which does not take an argument
	virtual void readOption(int argc, char **argv)
	{
		opterr = 0; /// prevent the error messages from getopt

		int ch;
		while ((ch = getopt(argc, argv, _optstring.c_str())) > 0) {

			switch (ch) {

				case ':':
					throw ConfigError(
						std::string("option requires an argument -- ")
						+ (char) optopt);
					break;

				case '?':
					throw ConfigError(
						std::string("invalid option -- ") + (char) optopt);
					break;

				default: {
					std::string name = getName(ch);
					if (getType(name) == BOOL) {
						setValue(name, "yes");
						_override.insert(name);
					}
					else {
						setValue(name, optarg);
						_override.insert(name);
					}
					break;
				}
			}
		}
	}
private:
	void setType(const std::string &name, Type t)
	{
		std::map<std::string, Type>::iterator i = _type.find(name);
		if (i != _type.end()) {
			throw ConfigError("config value " + name 
				+ " is already defined");
		}
		_type[name] = t;
	}

	Type getType(const std::string &name)
	{
		std::map<std::string, Type>::iterator i = _type.find(name);
		if (i == _type.end()) {
			throw ConfigError("config value " + name 
				+ " is not defined");
		}
		return i->second;
	}

	std::string getName(char optch)
	{
		std::map<char, std::string>::iterator name = _optchar.find(optch);
		if (name == _optchar.end()) {
			throw ConfigError(std::string("option ") + optch
				+ " is not defined");
		}
		return name->second;
	}

	void setOptChar(const std::string &name, char ch)
	{
		std::map<char, std::string>::iterator i = _optchar.find(ch);
		if (i != _optchar.end()) {
			throw ConfigError("option character for " + name 
				+ "(" + ch + ") is already defined");
		}
		_optchar[ch] = name;
		_optstring += ch;

		if (getType(name) != BOOL) {
			_optstring += ':';
		}
	}

	bool isDefined(const std::string &name)
	{
		return (_override.find(name) != _override.end());
	}

	void setValue(const std::string &name, const std::string &svalue)
	{
		switch (getType(name)) {
		case STRING:
			*(_sValue[name]) = svalue;
			break;
		case INT:
			*(_iValue[name]) = atoi(svalue.c_str());
			break;
		case DOUBLE:
			*(_dValue[name]) = atof(svalue.c_str());
			break;
		case BOOL: {

			_bValueStr[name] = svalue;

			std::string lowered;
			for (std::string::const_iterator i = svalue.begin();
					i != svalue.end();
					i++) {
				lowered += (char) tolower(*i);
			}

			if (lowered == "yes" || lowered == "true") {
				*(_bValue[name]) = true;
			}
			else if (lowered == "no" || lowered == "false") {
				*(_bValue[name]) = false;
			}
			else {
				throw ConfigError("boolean config values must be either yes/no "
							"or true/false");
			}
			break;
		}
			
		case STRING_LIST:
			_slValue[name]->push_back(svalue);
			break;
		}
	}

private:
	template<class ValueType>
	void writeValue(
		std::ostream &ost,
		const std::string &header,
		const std::map<std::string, ValueType> &values,
		const std::string &name
	) const {
		typename std::map<std::string, ValueType>::const_iterator i
			= values.find(name);
		assert(i != values.end());
		ost << header << name << "\t\t" << i->second << std::endl;
	}

	template<class ValueType>
	void writeValue(
		std::ostream &ost,
		const std::string &header,
		const std::map<std::string, ValueType*> &values,
		const std::string &name
	) const {
		typename std::map<std::string, ValueType*>::const_iterator i
			= values.find(name);
		assert(i != values.end());
		ost << header << name << "\t\t" << *(i->second) << std::endl;
	}

	template<class ValueType>
	void writeValue(
		std::ostream &ost,
		const std::string &header,
		const std::map<std::string, std::vector<ValueType>*> &values,
		const std::string &name
	) const {
		typename std::map<std::string, std::vector<ValueType>*>
			::const_iterator i = values.find(name);
		assert(i != values.end());

		const std::vector<ValueType> &v = *(i->second);
		for (size_t j = 0; j < v.size(); j++) {
			ost << header << name << "\t\t" << v[j] << std::endl;
		}
	}

	template<class ValueType>
	std::string strRep(const ValueType &v) const
	{
		std::ostringstream oss;
		oss << v;
		return oss.str();
	}

	template<class ValueType>
	std::string strRep(const ValueType *v) const
	{
		std::ostringstream oss;
		oss << *v;
		return oss.str();
	}

	template<class ValueType>
	std::string strRep(const std::vector<ValueType> &v) const
	{
		if (v.empty()) {
			return "";
		}

		std::ostringstream oss;
		oss << "[ ";
		std::copy(v.begin(), v.end(),
			std::ostream_iterator<ValueType>(oss, " "));
		oss << " ]";
		return oss.str();
	}

	template<class ValueType>
	void printHelpLine(
		std::ostream &ost,
		const std::string &name,
		char opt,
		const ValueType &defaultValue,
		const std::string &
	) const {
		if (opt > 0) {
			ost << '-' << opt << ' ';
		}
		else {
			ost << "   ";
		}

		if (name.size() < 20) {
			ost << std::setiosflags(std::ios_base::left)
				<< std::setw(20) << std::setfill(' ')
				<< name;
		}
		else {
			ost << name << ' ';
		}

		std::string defaultValueStr = strRep(defaultValue);
		if (! defaultValueStr.empty()) {
			ost << "(default: " << defaultValueStr << ")";
		}

		ost << std::endl;
	}

	/// we do not show the config value names which begin with "__"
	/// in the help message
	bool isHidden(const std::string &name) const
	{
		return (name.find("__") == 0);
	}


private:

	std::map<std::string, Type> _type;
	std::map<char, std::string> _optchar;
	std::map<std::string, std::string> _desc;
	std::set<std::string> _override;

	std::map<std::string, std::string*> _sValue;
	std::map<std::string, int*> _iValue;
	std::map<std::string, double*> _dValue;
	std::map<std::string, bool*> _bValue;
	std::map<std::string, std::string> _bValueStr;
	std::map<std::string, std::vector<std::string>*> _slValue;

	std::string _optstring;
	std::vector<std::pair<std::string, Type> > _order;
};

inline
std::ostream &operator<<(std::ostream &ost, const ConfigBase &c)
{
	c.write(ost);
	return ost;
}

class ConfigWithHelp : public ConfigBase {
public:
	ConfigWithHelp(void)
		: __help(false)
	{
		define("__help", &__help, 'h');
	}

	void readOption(int argc, char **argv)
	{
		ConfigBase::readOption(argc, argv);

		if (__help) {
			help(std::cerr);
			exit(0);
		}
	}

private:
	bool __help;
};

#endif //  __MYLIB_CONFIG_H__
