#include "eve-server.h"
#include "admin/commands/ParameterValidator.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cmath>

namespace Commands {

    // EnhancedParameter method implementation
    EnhancedParameter& EnhancedParameter::MinValue(double min, const std::string& errorMsg) {
        std::string msg = errorMsg.empty() ? "Value must be at least " + std::to_string(min) : errorMsg;
        constraints.emplace_back(ConstraintType::MinValue, std::to_string(min), msg);
        return *this;
    }

    EnhancedParameter& EnhancedParameter::MaxValue(double max, const std::string& errorMsg) {
        std::string msg = errorMsg.empty() ? "Value must be at most " + std::to_string(max) : errorMsg;
        constraints.emplace_back(ConstraintType::MaxValue, std::to_string(max), msg);
        return *this;
    }

    EnhancedParameter& EnhancedParameter::Range(double min, double max, const std::string& errorMsg) {
        std::string msg = errorMsg.empty() ? "Value must be between " + std::to_string(min) + " and " + std::to_string(max) : errorMsg;
        constraints.emplace_back(ConstraintType::MinValue, std::to_string(min), msg);
        constraints.emplace_back(ConstraintType::MaxValue, std::to_string(max), msg);
        return *this;
    }

    EnhancedParameter& EnhancedParameter::MinLength(size_t min, const std::string& errorMsg) {
        std::string msg = errorMsg.empty() ? "Length must be at least " + std::to_string(min) + " characters" : errorMsg;
        constraints.emplace_back(ConstraintType::MinLength, std::to_string(min), msg);
        return *this;
    }

    EnhancedParameter& EnhancedParameter::MaxLength(size_t max, const std::string& errorMsg) {
        std::string msg = errorMsg.empty() ? "Length must be at most " + std::to_string(max) + " characters" : errorMsg;
        constraints.emplace_back(ConstraintType::MaxLength, std::to_string(max), msg);
        return *this;
    }

    EnhancedParameter& EnhancedParameter::Pattern(const std::string& regex, const std::string& errorMsg) {
        std::string msg = errorMsg.empty() ? "Value does not match required pattern" : errorMsg;
        constraints.emplace_back(ConstraintType::Pattern, regex, msg);
        return *this;
    }

    EnhancedParameter& EnhancedParameter::OneOf(const std::vector<std::string>& values, const std::string& errorMsg) {
        std::string enumStr;
        for (size_t i = 0; i < values.size(); ++i) {
            if (i > 0) enumStr += ",";
            enumStr += values[i];
        }
        
        std::string msg = errorMsg;
        if (msg.empty()) {
            msg = "Value must be one of: ";
            for (size_t i = 0; i < values.size(); ++i) {
                if (i > 0) msg += ", ";
                msg += "'" + values[i] + "'";
            }
        }
        
        constraints.emplace_back(ConstraintType::Enum, enumStr, msg);
        return *this;
    }

    EnhancedParameter& EnhancedParameter::Custom(std::function<bool(const std::string&)> validator, const std::string& errorMsg) {
        constraints.emplace_back(validator, errorMsg);
        return *this;
    }

    EnhancedParameter& EnhancedParameter::Example(const std::string& example) {
        examples.push_back(example);
        return *this;
    }

    // ParameterValidator implementation
    ParameterValidator::ParameterValidator() {
    }

    ParameterValidator::~ParameterValidator() {
    }

    ValidationResult ParameterValidator::ValidateParameters(const std::vector<EnhancedParameter>& paramDefs, 
                                                          const std::vector<std::string>& args) const {
        // Check required parameter count
        size_t requiredCount = 0;
        for (const auto& param : paramDefs) {
            if (param.required) {
                requiredCount++;
            }
        }
        
        if (args.size() < requiredCount) {
            std::string error = "Not enough arguments. Required: " + std::to_string(requiredCount) + ", provided: " + std::to_string(args.size());
            std::string suggestion = "Usage: " + GenerateUsageExample("command", paramDefs);
            return ValidationResult::Error(error, suggestion);
        }
        
        if (args.size() > paramDefs.size()) {
            std::string error = "Too many arguments. Expected: " + std::to_string(paramDefs.size()) + ", provided: " + std::to_string(args.size());
            return ValidationResult::Error(error);
        }
        
        // Validate each parameter
        for (size_t i = 0; i < args.size() && i < paramDefs.size(); ++i) {
            ValidationResult result = ValidateParameter(paramDefs[i], args[i]);
            if (!result.success) {
                std::string error = "Parameter '" + paramDefs[i].name + "': " + result.errorMessage;
                return ValidationResult::Error(error, result.suggestion);
            }
        }
        
        return ValidationResult::Success();
    }

    ValidationResult ParameterValidator::ValidateParameter(const EnhancedParameter& paramDef, 
                                                         const std::string& value) const {
        // Check for empty values
        if (value.empty()) {
            if (paramDef.required && paramDef.defaultValue.empty()) {
                return ValidationResult::Error("Parameter is required", GenerateSuggestion(paramDef, value));
            }
            return ValidationResult::Success();
        }
        
        // Type validation
        ValidationResult typeResult = ValidateType(paramDef.type, value);
        if (!typeResult.success) {
            return ValidationResult::Error(typeResult.errorMessage, GenerateSuggestion(paramDef, value));
        }
        
        // Constraint validation
        ValidationResult constraintResult = ValidateConstraints(paramDef.constraints, value);
        if (!constraintResult.success) {
            return ValidationResult::Error(constraintResult.errorMessage, GenerateSuggestion(paramDef, value));
        }
        
        return ValidationResult::Success();
    }

    ValidationResult ParameterValidator::ValidateType(ParameterType type, const std::string& value) const {
        switch (type) {
            case ParameterType::String:
                return ValidateString(value);
            case ParameterType::Integer:
                return ValidateInteger(value);
            case ParameterType::Float:
                return ValidateNumber(value);
            case ParameterType::Boolean:
                return ValidateBoolean(value);
            default:
                return ValidationResult::Error("Unknown parameter type");
        }
    }

    ValidationResult ParameterValidator::ValidateConstraints(const std::vector<ParameterConstraint>& constraints, 
                                                           const std::string& value) const {
        for (const auto& constraint : constraints) {
            ValidationResult result;
            
            switch (constraint.type) {
                case ConstraintType::MinValue:
                    result = ValidateMinValue(value, std::stod(constraint.value));
                    break;
                case ConstraintType::MaxValue:
                    result = ValidateMaxValue(value, std::stod(constraint.value));
                    break;
                case ConstraintType::MinLength:
                    result = ValidateMinLength(value, std::stoull(constraint.value));
                    break;
                case ConstraintType::MaxLength:
                    result = ValidateMaxLength(value, std::stoull(constraint.value));
                    break;
                case ConstraintType::Pattern:
                    result = ValidatePattern(value, constraint.value);
                    break;
                case ConstraintType::Enum:
                    result = ValidateEnum(value, constraint.value);
                    break;
                case ConstraintType::Custom:
                    if (constraint.customValidator && !constraint.customValidator(value)) {
                        result = ValidationResult::Error(constraint.errorMessage);
                    }
                    break;
            }
            
            if (!result.success) {
                return ValidationResult::Error(FormatConstraintError(constraint, value));
            }
        }
        
        return ValidationResult::Success();
    }

    std::string ParameterValidator::GenerateParameterHelp(const std::vector<EnhancedParameter>& paramDefs) const {
        if (paramDefs.empty()) {
            return "No parameters required.";
        }
        
        std::string help = "Parameters:\n";
        
        for (const auto& param : paramDefs) {
            help += "  " + FormatParameterName(param) + " - " + param.description;
            
            if (!param.defaultValue.empty()) {
                help += " (default: " + param.defaultValue + ")";
            }
            
            if (!param.examples.empty()) {
                help += " [e.g., " + param.examples[0] + "]";
            }
            
            help += "\n";
        }
        
        return help;
    }

    std::string ParameterValidator::GenerateUsageExample(const std::string& commandName, 
                                                       const std::vector<EnhancedParameter>& paramDefs) const {
        std::string usage = "/" + commandName;
        
        for (const auto& param : paramDefs) {
            usage += " ";
            if (!param.required) {
                usage += "[";
            }
            usage += "<" + param.name + ">";
            if (!param.required) {
                usage += "]";
            }
        }
        
        return usage;
    }

    bool ParameterValidator::TryParseInteger(const std::string& value, int64& result) const {
        try {
            result = std::stoll(value);
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }

    bool ParameterValidator::TryParseNumber(const std::string& value, double& result) const {
        try {
            result = std::stod(value);
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }

    bool ParameterValidator::TryParseBoolean(const std::string& value, bool& result) const {
        std::string lower = value;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        
        if (lower == "true" || lower == "1" || lower == "yes" || lower == "on") {
            result = true;
            return true;
        } else if (lower == "false" || lower == "0" || lower == "no" || lower == "off") {
            result = false;
            return true;
        }
        
        return false;
    }

    std::string ParameterValidator::GenerateSuggestion(const EnhancedParameter& paramDef, const std::string& invalidValue) const {
        if (!paramDef.examples.empty()) {
            return "Try: " + paramDef.examples[0];
        }
        
        switch (paramDef.type) {
            case ParameterType::Integer:
                return "Expected an integer (e.g., 123)";
            case ParameterType::Float:
                return "Expected a number (e.g., 123.45)";
            case ParameterType::Boolean:
                return "Expected true/false, 1/0, yes/no, or on/off";
            case ParameterType::String:
            default:
                return "Expected a text value";
        }
    }

    // Internal validation methods
    ValidationResult ParameterValidator::ValidateString(const std::string& value) const {
        return ValidationResult::Success();
    }

    ValidationResult ParameterValidator::ValidateInteger(const std::string& value) const {
        int64 result;
        if (!TryParseInteger(value, result)) {
            return ValidationResult::Error("Invalid integer value: '" + value + "'");
        }
        return ValidationResult::Success();
    }

    ValidationResult ParameterValidator::ValidateNumber(const std::string& value) const {
        double result;
        if (!TryParseNumber(value, result)) {
            return ValidationResult::Error("Invalid number value: '" + value + "'");
        }
        return ValidationResult::Success();
    }

    ValidationResult ParameterValidator::ValidateBoolean(const std::string& value) const {
        bool result;
        if (!TryParseBoolean(value, result)) {
            return ValidationResult::Error("Invalid boolean value: '" + value + "'. Use true/false, 1/0, yes/no, or on/off");
        }
        return ValidationResult::Success();
    }

    ValidationResult ParameterValidator::ValidateMinValue(const std::string& value, double min) const {
        double numValue;
        if (!TryParseNumber(value, numValue)) {
            return ValidationResult::Error("Cannot validate minimum value for non-numeric input");
        }
        
        if (numValue < min) {
            return ValidationResult::Error("Value " + value + " is below minimum " + std::to_string(min));
        }
        
        return ValidationResult::Success();
    }

    ValidationResult ParameterValidator::ValidateMaxValue(const std::string& value, double max) const {
        double numValue;
        if (!TryParseNumber(value, numValue)) {
            return ValidationResult::Error("Cannot validate maximum value for non-numeric input");
        }
        
        if (numValue > max) {
            return ValidationResult::Error("Value " + value + " exceeds maximum " + std::to_string(max));
        }
        
        return ValidationResult::Success();
    }

    ValidationResult ParameterValidator::ValidateMinLength(const std::string& value, size_t min) const {
        if (value.length() < min) {
            return ValidationResult::Error("Length " + std::to_string(value.length()) + " is below minimum " + std::to_string(min));
        }
        return ValidationResult::Success();
    }

    ValidationResult ParameterValidator::ValidateMaxLength(const std::string& value, size_t max) const {
        if (value.length() > max) {
            return ValidationResult::Error("Length " + std::to_string(value.length()) + " exceeds maximum " + std::to_string(max));
        }
        return ValidationResult::Success();
    }

    ValidationResult ParameterValidator::ValidatePattern(const std::string& value, const std::string& pattern) const {
        try {
            auto it = m_regexCache.find(pattern);
            if (it == m_regexCache.end()) {
                m_regexCache[pattern] = std::regex(pattern);
                it = m_regexCache.find(pattern);
            }
            
            if (!std::regex_match(value, it->second)) {
                return ValidationResult::Error("Value '" + value + "' does not match required pattern");
            }
        } catch (const std::exception& e) {
            return ValidationResult::Error("Pattern validation error: " + std::string(e.what()));
        }
        
        return ValidationResult::Success();
    }

    ValidationResult ParameterValidator::ValidateEnum(const std::string& value, const std::string& enumValues) const {
        auto values = SplitEnumValues(enumValues);
        
        for (const auto& validValue : values) {
            if (value == validValue) {
                return ValidationResult::Success();
            }
        }
        
        std::string error = "Invalid value '" + value + "'. Valid options: ";
        for (size_t i = 0; i < values.size(); ++i) {
            if (i > 0) error += ", ";
            error += "'" + values[i] + "'";
        }
        
        return ValidationResult::Error(error);
    }

    std::string ParameterValidator::FormatParameterName(const EnhancedParameter& param) const {
        std::string name = param.name;
        if (!param.required) {
            name = "[" + name + "]";
        }
        return name;
    }

    std::string ParameterValidator::FormatConstraintError(const ParameterConstraint& constraint, const std::string& value) const {
        if (!constraint.errorMessage.empty()) {
            return constraint.errorMessage;
        }
        return "Constraint validation failed for value: " + value;
    }

    std::vector<std::string> ParameterValidator::SplitEnumValues(const std::string& enumString) const {
        std::vector<std::string> values;
        std::stringstream ss(enumString);
        std::string item;
        
        while (std::getline(ss, item, ',')) {
            // Trim leading and trailing spaces
            item.erase(0, item.find_first_not_of(" \t"));
            item.erase(item.find_last_not_of(" \t") + 1);
            if (!item.empty()) {
                values.push_back(item);
            }
        }
        
        return values;
    }

    // ParameterBuilder implementation
    EnhancedParameter ParameterBuilder::String(const std::string& name, const std::string& description) {
        return EnhancedParameter(name, ParameterType::String, true, description, "");
    }

    EnhancedParameter ParameterBuilder::Integer(const std::string& name, const std::string& description) {
        return EnhancedParameter(name, ParameterType::Integer, true, description, "");
    }

    EnhancedParameter ParameterBuilder::Number(const std::string& name, const std::string& description) {
        return EnhancedParameter(name, ParameterType::Float, true, description, "");
    }

    EnhancedParameter ParameterBuilder::Boolean(const std::string& name, const std::string& description) {
        return EnhancedParameter(name, ParameterType::Boolean, true, description, "");
    }

    EnhancedParameter ParameterBuilder::CharacterName(const std::string& name) {
        return String(name, "Character name")
            .MinLength(3).MaxLength(37)
            .Pattern("^[a-zA-Z0-9 '-]+$", "Character name contains invalid characters")
            .Example("John Doe");
    }

    EnhancedParameter ParameterBuilder::ItemTypeID(const std::string& name) {
        return Integer(name, "Item type ID")
            .MinValue(1).MaxValue(999999999)
            .Example("34");
    }

    EnhancedParameter ParameterBuilder::Quantity(const std::string& name) {
        return Integer(name, "Quantity")
            .MinValue(1).MaxValue(1000000)
            .Example("100");
    }

    EnhancedParameter ParameterBuilder::ISKAmount(const std::string& name) {
        return Number(name, "ISK amount")
            .MinValue(0.01).MaxValue(999999999999.99)
            .Example("1000000");
    }

    EnhancedParameter ParameterBuilder::SystemID(const std::string& name) {
        return Integer(name, "Solar system ID")
            .MinValue(30000001).MaxValue(32000000)
            .Example("30000142");
    }

    // ValidatorFactory implementation
    std::function<bool(const std::string&)> ValidatorFactory::CharacterNameValidator() {
        return [](const std::string& value) {
            return value.length() >= 3 && value.length() <= 37;
        };
    }

    std::function<bool(const std::string&)> ValidatorFactory::ItemTypeValidator() {
        return [](const std::string& value) {
            try {
                int64 typeID = std::stoll(value);
                return typeID > 0 && typeID <= 999999999;
            } catch (...) {
                return false;
            }
        };
    }

    std::function<bool(const std::string&)> ValidatorFactory::SystemIDValidator() {
        return [](const std::string& value) {
            try {
                int64 systemID = std::stoll(value);
                return systemID >= 30000001 && systemID <= 32000000;
            } catch (...) {
                return false;
            }
        };
    }

    std::function<bool(const std::string&)> ValidatorFactory::CoordinateValidator() {
        return [](const std::string& value) {
            try {
                double coord = std::stod(value);
                return std::isfinite(coord) && std::abs(coord) <= 1e15;
            } catch (...) {
                return false;
            }
        };
    }

    std::function<bool(const std::string&)> ValidatorFactory::ISKAmountValidator() {
        return [](const std::string& value) {
            try {
                double amount = std::stod(value);
                return amount >= 0.01 && amount <= 999999999999.99;
            } catch (...) {
                return false;
            }
        };
    }

    std::function<bool(const std::string&)> ValidatorFactory::RangeValidator(double min, double max) {
        return [min, max](const std::string& value) {
            try {
                double num = std::stod(value);
                return num >= min && num <= max;
            } catch (...) {
                return false;
            }
        };
    }

    std::function<bool(const std::string&)> ValidatorFactory::LengthValidator(size_t min, size_t max) {
        return [min, max](const std::string& value) {
            return value.length() >= min && value.length() <= max;
        };
    }

    std::function<bool(const std::string&)> ValidatorFactory::RegexValidator(const std::string& pattern) {
        return [pattern](const std::string& value) {
            try {
                std::regex regex(pattern);
                return std::regex_match(value, regex);
            } catch (...) {
                return false;
            }
        };
    }

} // namespace Commands