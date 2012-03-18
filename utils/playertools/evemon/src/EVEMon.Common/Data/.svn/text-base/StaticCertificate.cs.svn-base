using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;
using System.Xml;
using EVEMon.Common.Serialization.Datafiles;

namespace EVEMon.Common.Data
{
	/// <summary>
	/// Represents a certificate from the datafiles.
	/// Every category (i.e. "Business and Industry")
	/// contains certificate classes (i.e. "Production Manager"),
	/// which contain certificates (i.e. "Production Manager Basic").
	/// </summary>
	public sealed class StaticCertificate
	{
		private readonly StaticRecommendations<Item> m_recommendations = new StaticRecommendations<Item>();
		private readonly List<StaticCertificate> m_prerequisiteCertificates = new List<StaticCertificate>();
		private readonly List<StaticSkillLevel> m_prerequisiteSkills = new List<StaticSkillLevel>();

		public readonly long m_id;
		public readonly string m_description;
		public readonly CertificateGrade m_grade;
		public readonly StaticCertificateClass m_class;

		/// <summary>
		/// Constructor from XML
		/// </summary>
		/// <param name="certClass"></param>
		/// <param name="element"></param>
		internal StaticCertificate(StaticCertificateClass certClass, SerializableCertificate src)
		{
			m_class = certClass;

			m_id = src.ID;
			m_grade = src.Grade;
			m_description = src.Description;

			// Recommendations
			if (src.Recommendations != null)
			{
				foreach (var recommendation in src.Recommendations)
				{
					var ship = StaticItems.Ships.AllItems.FirstOrDefault(x => x.Name == recommendation.Ship) as Ship;
					if (ship != null)
					{
						ship.Recommendations.Add(this);
						m_recommendations.Add(ship);
					}
				}
			}
		}

		/// <summary>
		/// Completes the initialization by updating the prerequisites
		/// </summary>
		internal void CompleteInitialization(SerializableCertificatePrerequisite[] prereqs)
		{
			foreach(var prereq in prereqs)
			{
				// Skills
				if (prereq.Kind == SerializableCertificatePrerequisiteKind.Skill)
				{
					m_prerequisiteSkills.Add(new StaticSkillLevel(prereq.Name, Int32.Parse(prereq.Level)));
				}
				// Certificates
				else
				{
					var grade = StaticCertificate.GetGrade(prereq.Level);
					m_prerequisiteCertificates.Add(StaticCertificates.GetCertificateClass(prereq.Name)[grade]);
				}
			}
		}

		/// <summary>
		/// Gets this certificate's ID
		/// </summary>
		public long ID
		{
			get { return m_id; }
		}

		/// <summary>
		/// Gets this certificate's name
		/// </summary>
		public string Name
		{
			get { return m_class.Name; }
		}

		/// <summary>
		/// Gets the certificate's class.
		/// </summary>
		public StaticCertificateClass Class
		{
			get { return m_class; }
		}

		/// <summary>
		/// Gets this certificate's description
		/// </summary>
		public string Description
		{
			get { return m_description; }
		}

		/// <summary>
		/// Gets this certificate's grade
		/// </summary>
		public CertificateGrade Grade
		{
			get { return m_grade; }
		}

		/// <summary>
		/// Gets the ships this certificate is recommended for
		/// </summary>
		public StaticRecommendations<Item> Recommendations
		{
			get { return m_recommendations; }
		}

		/// <summary>
		/// Gets the grade from the provided grade key. No need to previously interns the key, it will be itnerned in this method
		/// </summary>
		/// <param name="key"></param>
		/// <returns></returns>
		private static CertificateGrade GetGrade(string key)
		{
			switch (key)
			{
				case "Basic":
					return CertificateGrade.Basic;
				case "Standard":
					return CertificateGrade.Standard;
				case "Improved":
					return CertificateGrade.Improved;
				case "Elite":
					return CertificateGrade.Elite;
				default:
					throw new NotImplementedException();
			}
		}

		/// <summary>
		/// Gets the prerequisite skills
		/// </summary>
		public IEnumerable<StaticSkillLevel> PrerequisiteSkills
		{
			get { return m_prerequisiteSkills; }
		}

		/// <summary>
		/// Gets the prerequisite certificates
		/// </summary>
		public IEnumerable<StaticCertificate> PrerequisiteCertificates
		{
			get { return m_prerequisiteCertificates; }
		}

		/// <summary>
		/// Gets all the top-level prerequisite skills, including the ones from prerequisite certificates.
		/// </summary>
		public IEnumerable<StaticSkillLevel> AllTopPrerequisiteSkills
		{
			get
			{
				int[] highestLevels = new int[StaticSkills.ArrayIndicesCount];
				List<StaticSkillLevel> list = new List<StaticSkillLevel>();

				// Collect all top prerequisites from certificates
				foreach (var certPrereq in m_prerequisiteCertificates)
				{
					foreach (var certSkilPrereq in certPrereq.AllTopPrerequisiteSkills)
					{
						if (highestLevels[certSkilPrereq.Skill.ArrayIndex] < certSkilPrereq.Level)
						{
							highestLevels[certSkilPrereq.Skill.ArrayIndex] = certSkilPrereq.Level;
							list.Add(certSkilPrereq);
						}
					}
				}

				// Collect all prerequisites from skills
				foreach (var skillPrereq in m_prerequisiteSkills)
				{
					if (highestLevels[skillPrereq.Skill.ArrayIndex] < skillPrereq.Level)
					{
						highestLevels[skillPrereq.Skill.ArrayIndex] = skillPrereq.Level;
						list.Add(skillPrereq);
					}
				}

				// Return the result
				foreach (var newItem in list)
				{
					if (highestLevels[newItem.Skill.ArrayIndex] != 0)
					{
						yield return new StaticSkillLevel(newItem.Skill, highestLevels[newItem.Skill.ArrayIndex]);
						highestLevels[newItem.Skill.ArrayIndex] = 0;
					}
				}
			}
		}

		/// <summary>
		/// Gets a string representation of this certificate.
		/// </summary>
		/// <returns></returns>
		public override string ToString()
		{
			return this.m_class.Name + " " + this.m_grade.ToString();
		}

		/// <summary>
		/// Gets the equivalent representation of this certificate for the given character.
		/// </summary>
		/// <param name="character"></param>
		/// <returns></returns>
		public Certificate ToCharacter(Character character)
		{
			return character.Certificates[this.ID];
		}
	}
}

