#ifndef __PARAMETERVALIDATOR_H_INCL__
#define __PARAMETERVALIDATOR_H_INCL__

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <regex>
#include "ICommand.h"

namespace Commands {

    // Parameter validation result
    struct ValidationResult {
        bool success;
        std::string errorMessage;
        std::string suggestion;
        
        ValidationResult(bool success = true, const std::string& error = "", const std::string& suggestion = "")
            : success(success), errorMessage(error), suggestion(suggestion) {}
        
        static ValidationResult Success() {
            return ValidationResult(true);
        }
        
        static ValidationResult Error(const std::string& message, const std::string& suggestion = "") {
            return ValidationResult(false, message, suggestion);
        }
    };

    // Parameter constraint types
    enum class ConstraintType {
        MinValue,
        MaxValue,
        MinLength,
        MaxLength,
        Pattern,
        Enum,
        Custom
    };

    // Parameter constraint definition
    struct ParameterConstraint {
        ConstraintType type;
        std::string value;
        std::string errorMessage;
        std::function<bool(const std::string&)> customValidator;
        
        ParameterConstraint(ConstraintType type, const std::string& value, const std::string& errorMessage = "")
            : type(type), value(value), errorMessage(errorMessage) {}
        
        ParameterConstraint(std::function<bool(const std::string&)> validator, const std::string& errorMessage)
            : type(ConstraintType::Custom), errorMessage(errorMessage), customValidator(validator) {}
    };

    // Enhanced parameter definition
    struct EnhancedParameter {
        std::string name;
        ParameterType type;
        bool required;
        std::string description;
        std::string defaultValue;
        std::vector<ParameterConstraint> constraints;
        std::vector<std::string> examples;
        
        EnhancedParameter(const std::string& name, ParameterType type, bool required = true, 
                         const std::string& description = "", const std::string& defaultValue = "")
            : name(name), type(type), required(required), description(description), defaultValue(defaultValue) {}
        
        // Convenient methods for adding constraints
        EnhancedParameter& MinValue(double min, const std::string& errorMsg = "");
        EnhancedParameter& MaxValue(double max, const std::string& errorMsg = "");
        EnhancedParameter& Range(double min, double max, const std::string& errorMsg = "");
        EnhancedParameter& MinLength(size_t min, const std::string& errorMsg = "");
        EnhancedParameter& MaxLength(size_t max, const std::string& errorMsg = "");
        EnhancedParameter& Pattern(const std::string& regex, const std::string& errorMsg = "");
        EnhancedParameter& OneOf(const std::vector<std::string>& values, const std::string& errorMsg = "");
        EnhancedParameter& Custom(std::function<bool(const std::string&)> validator, const std::string& errorMsg);
        EnhancedParameter& Example(const std::string& example);
    };

    // Parameter validator
    class ParameterValidator {
    public:
        ParameterValidator();
        ~ParameterValidator();
        
        // Validate parameter list
        ValidationResult ValidateParameters(const std::vector<EnhancedParameter>& paramDefs, 
                                          const std::vector<std::string>& args) const;
        
        // Validate single parameter
        ValidationResult ValidateParameter(const EnhancedParameter& paramDef, 
                                         const std::string& value) const;
        
        // Type validation
        ValidationResult ValidateType(ParameterType type, const std::string& value) const;
        
        // Constraint validation
        ValidationResult ValidateConstraints(const std::vector<ParameterConstraint>& constraints, 
                                            const std::string& value) const;
        
        // Generate parameter help
        std::string GenerateParameterHelp(const std::vector<EnhancedParameter>& paramDefs) const;
        
        // Generate usage examples
        std::string GenerateUsageExample(const std::string& commandName, 
                                        const std::vector<EnhancedParameter>& paramDefs) const;
        
        // Type conversion and validation
        bool TryParseInteger(const std::string& value, int64& result) const;
        bool TryParseNumber(const std::string& value, double& result) const;
        bool TryParseBoolean(const std::string& value, bool& result) const;
        
        // Suggestion generation
        std::string GenerateSuggestion(const EnhancedParameter& paramDef, const std::string& invalidValue) const;
        
    private:
        // Internal validation methods
        ValidationResult ValidateString(const std::string& value) const;
        ValidationResult ValidateInteger(const std::string& value) const;
        ValidationResult ValidateNumber(const std::string& value) const;
        ValidationResult ValidateBoolean(const std::string& value) const;
        
        // Constraint validation methods
        ValidationResult ValidateMinValue(const std::string& value, double min) const;
        ValidationResult ValidateMaxValue(const std::string& value, double max) const;
        ValidationResult ValidateMinLength(const std::string& value, size_t min) const;
        ValidationResult ValidateMaxLength(const std::string& value, size_t max) const;
        ValidationResult ValidatePattern(const std::string& value, const std::string& pattern) const;
        ValidationResult ValidateEnum(const std::string& value, const std::string& enumValues) const;
        
        // Helper methods
        std::string FormatParameterName(const EnhancedParameter& param) const;
        std::string FormatConstraintError(const ParameterConstraint& constraint, const std::string& value) const;
        std::vector<std::string> SplitEnumValues(const std::string& enumString) const;
        
        // Cached regular expressions
        mutable std::map<std::string, std::regex> m_regexCache;
    };

    // Parameter builder - for fluent parameter definition construction
    class ParameterBuilder {
    public:
        static EnhancedParameter String(const std::string& name, const std::string& description = "");
        static EnhancedParameter Integer(const std::string& name, const std::string& description = "");
        static EnhancedParameter Number(const std::string& name, const std::string& description = "");
        static EnhancedParameter Boolean(const std::string& name, const std::string& description = "");
        
        // Predefined common parameter types
        static EnhancedParameter CharacterName(const std::string& name = "character");
        static EnhancedParameter ItemTypeID(const std::string& name = "typeID");
        static EnhancedParameter Quantity(const std::string& name = "quantity");
        static EnhancedParameter ISKAmount(const std::string& name = "amount");
        static EnhancedParameter SystemID(const std::string& name = "systemID");
        static EnhancedParameter Coordinates(const std::string& prefix = "");
    };

    // Validator factory
    class ValidatorFactory {
    public:
        // Common validators
        static std::function<bool(const std::string&)> CharacterNameValidator();
        static std::function<bool(const std::string&)> ItemTypeValidator();
        static std::function<bool(const std::string&)> SystemIDValidator();
        static std::function<bool(const std::string&)> CoordinateValidator();
        static std::function<bool(const std::string&)> ISKAmountValidator();
        
        // Generic validators
        static std::function<bool(const std::string&)> RangeValidator(double min, double max);
        static std::function<bool(const std::string&)> LengthValidator(size_t min, size_t max);
        static std::function<bool(const std::string&)> RegexValidator(const std::string& pattern);
    };

} // namespace Commands

#endif // __PARAMETERVALIDATOR_H_INCL__