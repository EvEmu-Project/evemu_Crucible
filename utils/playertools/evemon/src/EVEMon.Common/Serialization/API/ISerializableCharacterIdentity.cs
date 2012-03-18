namespace EVEMon.Common.Serialization
{
    /// <summary>
    /// Represents a set of informations required to create an identity
    /// </summary>
    public interface ISerializableCharacterIdentity
    {
        long ID { get; }
        string Name { get; }
    }
}
