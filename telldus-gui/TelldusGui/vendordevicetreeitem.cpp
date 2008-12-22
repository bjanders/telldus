#include "vendordevicetreeitem.h"

#include <QFile>
#include <QPixmap>
#include <QDebug>
#include <iostream>

VendorDeviceTreeItem::VendorDeviceTreeItem(int id, VendorDeviceTreeItem *parent)
		:deviceId(id),
		parentItem(parent)
{
}

VendorDeviceTreeItem::~VendorDeviceTreeItem() {
	qDeleteAll(childItems);
}

void VendorDeviceTreeItem::appendChild(VendorDeviceTreeItem *item) {
	childItems.append(item);
}

VendorDeviceTreeItem *VendorDeviceTreeItem::child(int row) {
	return childItems.value(row);
}

int VendorDeviceTreeItem::childCount() const {
	return childItems.count();
}
int VendorDeviceTreeItem::columnCount() const {
	return 1;
}

QVariant VendorDeviceTreeItem::data(int column) const {
	if (column == 0) {
		return deviceName;
	}

	return QVariant();
}

VendorDeviceTreeItem *VendorDeviceTreeItem::parent() {
	return parentItem;
}

int VendorDeviceTreeItem::row() const {
	if (parentItem) {
		return parentItem->childItems.indexOf(const_cast<VendorDeviceTreeItem*>(this));
	}

	return 0;
}

QPixmap VendorDeviceTreeItem::image() const {
	QString filename;
	if (deviceId == 0) {
		filename = ":/images/vendors/" + img.toLower() + ".jpg";
	} else {
		filename = ":/images/devices/" + img + ".jpg";
	}
	QPixmap image = QPixmap(filename);
	if (image.isNull()) {
		image = QPixmap( 96, 96 );
		image.fill( Qt::transparent );
	}
	return image;
}

bool VendorDeviceTreeItem::parseXml( const QString &filename ) {
	QFile file(filename);
	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		std::cerr << "Error: Cannot read file " << qPrintable(filename)
				  << ": " << qPrintable(file.errorString())
				  << std::endl;
		return false;
	}

	QXmlStreamReader reader;
	reader.setDevice(&file);

	reader.readNext();
	while (!reader.atEnd()) {
		if (reader.isStartElement()) {
			if (reader.name() == "devices") { //First element
				reader.readNext();
			} else if (reader.name() == "vendor") {
				parseVendor( &reader );
			} else {
				reader.raiseError( QObject::tr("Not a Telldus device-file"));
			}
		} else {
			reader.readNext();
		}
	}

	file.close();
	if (reader.hasError()) {
		std::cerr << "Error: Failed to parse file "
				  << qPrintable(filename) << ": "
				  << qPrintable(reader.errorString()) << std::endl;
		return false;
	} else if (file.error() != QFile::NoError) {
		std::cerr << "Error: Cannot read file " << qPrintable(filename)
				  << ": " << qPrintable(file.errorString())
				  << std::endl;
		return false;
	}

	return true;
}

void VendorDeviceTreeItem::parseVendor( QXmlStreamReader *reader ) {
	VendorDeviceTreeItem *item = new VendorDeviceTreeItem(0, this);
	item->deviceName = reader->attributes().value("name").toString();
	item->img = reader->attributes().value("name").toString();
	appendChild(item);

	reader->readNext();
	while(!reader->atEnd()) {
		if (reader->isEndElement()) {
			reader->readNext();
			break;
		}

		if (reader->isStartElement()) {
			if (reader->name() == "device") {
				parseDevice( reader, item );
			} else {
				skipUnknownElement( reader );
			}
		} else {
			reader->readNext();
		}
	}
}

void VendorDeviceTreeItem::parseDevice( QXmlStreamReader *reader, VendorDeviceTreeItem *parent ) {
	VendorDeviceTreeItem *item = new VendorDeviceTreeItem(reader->attributes().value("id").toString().toInt(), parent);
	item->img = reader->attributes().value("image").toString();
	item->deviceName = reader->readElementText(); //This call must be the last one because it clears the attribute-list
	parent->appendChild(item);

	while(!reader->atEnd()) {
		if (reader->isEndElement()) {
			reader->readNext();
			break;
		}

		if (reader->isStartElement()) {
			skipUnknownElement( reader );
		} else {
			reader->readNext();
		}
	}
}

void VendorDeviceTreeItem::skipUnknownElement( QXmlStreamReader *reader ) {
	reader->readNext();
	while (!reader->atEnd()) {
		if (reader->isEndElement()) {
			reader->readNext();
			break;
		}

		if (reader->isStartElement()) {
			skipUnknownElement( reader );
		} else {
			reader->readNext();
		}
	}
}