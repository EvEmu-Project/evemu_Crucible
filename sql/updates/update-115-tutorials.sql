CREATE TABLE tutorial_categories (
	categoryID int(10) unsigned NOT NULL,
	categoryName varchar(100) NOT NULL,
	description varchar(200) NOT NULL default '',
	PRIMARY KEY (categoryID)
	) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO tutorial_categories
(categoryID, categoryName, description)
VALUES
(1, 'Beginner Tutorials', 'Here are the basic tutorials for your first days in EVE!'),
(2, 'Intermediate Tutorials', 'Under this category fall all advanced tutorials'),
(3, 'Advanced Tutorials', ''),
(4, 'Informative Tutorials', '');

ALTER TABLE tutorials
 CHANGE tutorialVName tutorialName varchar(100) NOT NULL,
 ADD COLUMN categoryID int(10) unsigned NOT NULL;

-- hacked
UPDATE tutorials
SET categoryID = 1;
