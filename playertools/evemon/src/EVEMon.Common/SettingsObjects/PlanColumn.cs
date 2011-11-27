using System.ComponentModel;
using EVEMon.Common.Attributes;

namespace EVEMon.Common.SettingsObjects
{
    /// <summary>
    /// Represents the available column types
    /// </summary>
    public enum PlanColumn
    {
        None = -1,

        [Header("Skill Name")]
        [Description("Skill Name")]
        SkillName = 0,

        [Header("Plan Group")]
        [Description("Plan Group")]
        PlanGroup = 1,

        [Header("Training Time")]
        [Description("Training Time")]
        TrainingTime = 2,

        [Header("Training Time (no implants)")]
        [Description("Training Time (no implants)")]
        TrainingTimeNatural = 3,

        [Header("Earliest Start")]
        [Description("Earliest Start")]
        EarliestStart = 4,

        [Header("Earliest End")]
        [Description("Earliest End")]
        EarliestEnd = 5,

        [Header("Conflicts")]
        [Description("Conflicts")]
        Conflicts = 6,

        [Header("%")]
        [Description("Percent Complete")]
        PercentComplete = 7,

        [Header("Rank")]
        [Description("Skill Rank")]
        SkillRank = 8,

        [Header("Primary")]
        [Description("Primary Attribute")]
        PrimaryAttribute = 9,

        [Header("Secondary")]
        [Description("Secondary Attribute")]
        SecondaryAttribute = 10,

        [Header("Group")]
        [Description("Skill Group")]
        SkillGroup = 11,

        [Header("Notes")]
        [Description("Notes")]
        Notes = 12,

        [Header("Type")]
        [Description("Plan Type (Planned/Prerequisite)")]
        PlanType = 13,

        [Header("Est. SP Total")]
        [Description("Estimated Skill Point Total")]
        SPTotal = 14,

        [Header("SP/Hour")]
        [Description("SP/Hour")]
        SPPerHour = 15,

        [Header("Priority")]
        [Description("Priority")]
        Priority = 16,

        [Header("Cost")]
        [Description("Cost")]
        Cost = 17,

        [Header("Skill Points Required")]
        [Description("Skill Points Required")]
        SkillPointsRequired = 18
    }
}
