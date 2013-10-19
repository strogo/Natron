//  Powiter
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/*
*Created by Alexandre GAUTHIER-FOICHAT on 6/1/2012. 
*contact: immarespond at gmail dot com
*
*/

#ifndef POWITER_GLOBAL_APPMANAGER_H_
#define POWITER_GLOBAL_APPMANAGER_H_

#include <vector>
#include <string>
#include <map>

#include <QtCore/QObject>
#include <QtCore/QDateTime>
#include <QtCore/QStringList>


#ifndef Q_MOC_RUN
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#endif

#include "Global/GlobalDefines.h"

#include "Engine/Singleton.h"
#include "Engine/Row.h"
#include "Engine/FrameEntry.h"
#include "Engine/Format.h"

/*macro to get the unique pointer to the controler*/
#define appPTR AppManager::instance()

class KnobFactory;
class NodeGui;
class Node;
class Model;
class ViewerNode;
class Writer;
class ViewerTab;
class TabWidget;
class Gui;
class VideoEngine;
class QMutex;
class OutputNodeInstance;
class TimeLine;
namespace Powiter {
    class LibraryBinary;
    class OfxHost;
}

class Project{
    QString _projectName;
    QString _projectPath;
    bool _hasProjectBeenSavedByUser;
    QDateTime _ageSinceLastSave;
    QDateTime _lastAutoSave;
    Format* _format; // project default format FIXME: make this shared_ptr
    boost::shared_ptr<TimeLine> _timeline; // global timeline

    
public:
    Project();
    
    ~Project(){}
    
    const QString& getProjectName() const WARN_UNUSED_RETURN {return _projectName;}
    
    void setProjectName(const QString& name) {_projectName = name;}
    
    const QString& getProjectPath() const WARN_UNUSED_RETURN {return _projectPath;}
    
    void setProjectPath(const QString& path) {_projectPath = path;}
    
    bool hasProjectBeenSavedByUser() const WARN_UNUSED_RETURN {return _hasProjectBeenSavedByUser;}
    
    void setHasProjectBeenSavedByUser(bool s) {_hasProjectBeenSavedByUser = s;}
    
    const QDateTime& projectAgeSinceLastSave() const WARN_UNUSED_RETURN {return _ageSinceLastSave;}
    
    void setProjectAgeSinceLastSave(const QDateTime& t) {_ageSinceLastSave = t;}
    
    const QDateTime& projectAgeSinceLastAutosave() const WARN_UNUSED_RETURN {return _lastAutoSave;}
    
    void setProjectAgeSinceLastAutosaveSave(const QDateTime& t) {_lastAutoSave = t;}
    
    const Format& getProjectDefaultFormat() const WARN_UNUSED_RETURN {return *_format;}
    
    void setProjectDefaultFormat(Format* f) {_format = f;}
    
    boost::shared_ptr<TimeLine> getTimeLine() const WARN_UNUSED_RETURN {return _timeline;}
    
    // TimeLine operations (to avoid duplicating the shared_ptr when possible)
    void setFrameRange(int first, int last);
    
    void seekFrame(int frame);
    
    void incrementCurrentFrame();
    
    void decrementCurrentFrame();
    
    int currentFrame() const WARN_UNUSED_RETURN;
    
    int firstFrame() const WARN_UNUSED_RETURN;
    
    int lastFrame() const WARN_UNUSED_RETURN;

};

/*Controler (see Model-view-controler pattern on wikipedia). This class
 implements the singleton pattern to ensure there's only 1 single
 instance of the object living. Also you can access the controler
 by the handy macro appPTR*/
class AppInstance : public QObject
{
    Q_OBJECT
public:
    AppInstance(int appID,const QString& projectName = QString());
    
    ~AppInstance();
    
    int getAppID() const {return _appID;}

    /*Create a new node  in the node graph.
     The name passed in parameter must match a valid node name,
     otherwise an exception is thrown. You should encapsulate the call
     by a try-catch block.*/
    Node* createNode(const QString& name) WARN_UNUSED_RETURN;
    
    /*Pointer to the GUI*/
    Gui* getGui() WARN_UNUSED_RETURN {return _gui;}
    
    /* The following methods are forwarded to the model */
    void checkViewersConnection();
        
    const std::vector<Node*> getAllActiveNodes() const WARN_UNUSED_RETURN;
    
    const QString& getCurrentProjectName() const WARN_UNUSED_RETURN {return _currentProject.getProjectName();}
    
    const QString& getCurrentProjectPath() const WARN_UNUSED_RETURN {return _currentProject.getProjectPath();}
    
    boost::shared_ptr<TimeLine> getTimeLine() const WARN_UNUSED_RETURN {return _currentProject.getTimeLine();}
    
    void setCurrentProjectName(const QString& name) {_currentProject.setProjectName(name);}
    
    void loadProject(const QString& path,const QString& name);
    
    void saveProject(const QString& path,const QString& name,bool autoSave);
    
    void autoSave();
    
    bool hasProjectBeenSavedByUser() const WARN_UNUSED_RETURN {return _currentProject.hasProjectBeenSavedByUser();}
    
    const Format& getProjectFormat() const WARN_UNUSED_RETURN {return _currentProject.getProjectDefaultFormat();}
    
    void setProjectFormat(Format* frmt){_currentProject.setProjectDefaultFormat(frmt);}
    
    
    
    void resetCurrentProject();
    
    void clearNodes();
        
    bool isSaveUpToDate() const WARN_UNUSED_RETURN;
    
    void deselectAllNodes() const;
        
    static QString autoSavesDir() WARN_UNUSED_RETURN;
    
    ViewerTab* addNewViewerTab(ViewerNode* node,TabWidget* where) WARN_UNUSED_RETURN;
    
    bool connect(int inputNumber,const std::string& inputName,Node* output) WARN_UNUSED_RETURN;
    
    bool connect(int inputNumber,Node* input,Node* output) WARN_UNUSED_RETURN;
    
    bool disconnect(Node* input,Node* output) WARN_UNUSED_RETURN;
    
    void autoConnect(Node* target,Node* created);
    
    NodeGui* getNodeGui(Node* n) const WARN_UNUSED_RETURN;
    
    Node* getNode(NodeGui* n) const WARN_UNUSED_RETURN;
    
    void connectViewersToViewerCache();
    
    void disconnectViewersFromViewerCache();
    
    QMutex* getAutoSaveMutex() const WARN_UNUSED_RETURN {return _autoSaveMutex;}

    void errorDialog(const std::string& title,const std::string& message) const;
    
    void warningDialog(const std::string& title,const std::string& message) const;
    
    void informationDialog(const std::string& title,const std::string& message) const;
    
    Powiter::StandardButton questionDialog(const std::string& title,const std::string& message,Powiter::StandardButtons buttons =
                                           Powiter::StandardButtons(Powiter::Yes | Powiter::No),
                                           Powiter::StandardButton defaultButton = Powiter::NoButton) const WARN_UNUSED_RETURN;
    
    
    
public slots:

    
    
    void triggerAutoSave();
    
    void onRenderingOnDiskStarted(Writer* writer,const QString& sequenceName,int firstFrame,int lastFrame);


signals:
    
    void pluginsPopulated();

private:
    
    
	void removeAutoSaves() const;
    
    /*Attemps to find an autosave. If found one,prompts the user
     whether he/she wants to load it. If something was loaded this function
     returns true,otherwise false.*/
    bool findAutoSave() WARN_UNUSED_RETURN;
        
    boost::scoped_ptr<Model> _model; // the model of the MVC pattern
    Gui* _gui; // the view of the MVC pattern
    
    Project _currentProject;
    
    int _appID;
    
    std::map<Node*,NodeGui*> _nodeMapping;

    QMutex* _autoSaveMutex;
};

class AppManager : public QObject, public Singleton<AppManager>
{

    Q_OBJECT
    
public:
    
    class PluginToolButton{
    public:
        PluginToolButton(const QStringList& groups,
                         const QString& pluginName,
                         const QString& pluginIconPath,
                         const QString& groupIconPath):
        _groups(groups),
        _pluginName(pluginName),
        _pluginIconPath(pluginIconPath),
        _groupIconPath(groupIconPath)
        {
            
        }
        QStringList _groups;
        QString _pluginName;
        QString _pluginIconPath;
        QString _groupIconPath;
        
    };

    typedef std::map< std::string,std::pair< std::vector<std::string> ,Powiter::LibraryBinary*> >::iterator ReadPluginsIterator;
    typedef ReadPluginsIterator WritePluginsIterator;
    
    AppManager();
    
    virtual ~AppManager();
    
    const boost::scoped_ptr<Powiter::OfxHost>& getOfxHost() const WARN_UNUSED_RETURN {return ofxHost;}
    
    AppInstance* newAppInstance(const QString& projectName = QString());
    
    AppInstance* getAppInstance(int appID) const;
    
    void removeInstance(int appID);
    
    void setAsTopLevelInstance(int appID);
    
    AppInstance* getTopLevelInstance () const WARN_UNUSED_RETURN;
    
    /*Return a list of the name of all nodes available currently in the software*/
    const QStringList& getNodeNameList() const WARN_UNUSED_RETURN {return _nodeNames;}
    
    /*Find a builtin format with the same resolution and aspect ratio*/
    Format* findExistingFormat(int w, int h, double pixel_aspect = 1.0) const WARN_UNUSED_RETURN;
    
    const std::vector<PluginToolButton*>& getPluginsToolButtons() const WARN_UNUSED_RETURN {return _toolButtons;}
    
    /*Tries to load all plugins in directory "where"*/
    static std::vector<Powiter::LibraryBinary*> loadPlugins (const QString& where) WARN_UNUSED_RETURN;
    
    /*Tries to load all plugins in directory "where" that contains all the functions described by
     their name in "functions".*/
    static std::vector<Powiter::LibraryBinary*> loadPluginsAndFindFunctions(const QString& where,
                                                                            const std::vector<std::string>& functions) WARN_UNUSED_RETURN;
    
    const Powiter::Cache<Powiter::FrameEntry>& getViewerCache() const WARN_UNUSED_RETURN {return *_viewerCache;}
    
    const Powiter::Cache<Powiter::Row>& getNodeCache() const WARN_UNUSED_RETURN {return *_nodeCache;}
    
    const KnobFactory& getKnobFactory() const WARN_UNUSED_RETURN {return *_knobFactory;}
    
 
public slots:
    
    void clearPlaybackCache();
    
    void clearDiskCache();
    
    void clearNodeCache();
    
    void addPluginToolButtons(const QStringList& groups,
                                const QString& pluginName,
                                const QString& pluginIconPath,
                                const QString& groupIconPath);
    /*Quit the app*/
    static void quit();
private:
    
    /*Loads all kind of plugins*/
    void loadAllPlugins();
    
    //////////////////////////////
    //// NODE PLUGINS
    /* Viewer,Reader,Writer...etc.
     No function to load external plugins
     yet since the SDK isn't released.*/
    void loadBuiltinNodePlugins();
    //////////////////////////////
    
    //////////////////////////////
    //// READ PLUGINS
    /*loads extra reader plug-ins */
    void loadReadPlugins();
    /*loads reads that are incorporated to Powiter*/
    void loadBuiltinReads();
    //////////////////////////////

    //////////////////////////////
    //// WRITE PLUGINS
    /*loads extra writer plug-ins*/
    void loadWritePlugins();
    /*loads writes that are built-ins*/
    void loadBuiltinWrites();
    //////////////////////////////
    
    void loadBuiltinFormats();
    
    
    std::map<int,AppInstance*> _appInstances;
    
    int _availableID;
    
    int _topLevelInstanceID;
    
    /*map< decoder name, pair< vector<file type decoded>, decoder library> >*/
    std::map< std::string,std::pair< std::vector<std::string> ,Powiter::LibraryBinary*> > _readPluginsLoaded;
    
    /*map< encoder name, pair< vector<file type encoded>, encoder library> >*/
    std::map< std::string,std::pair< std::vector<std::string> ,Powiter::LibraryBinary*> > _writePluginsLoaded;
    
    std::vector<Format*> _formats;
    
    QStringList _nodeNames;
    
    boost::scoped_ptr<Powiter::OfxHost> ofxHost;
    
    std::vector<PluginToolButton*> _toolButtons;
    
    boost::scoped_ptr<KnobFactory> _knobFactory;
    
    boost::scoped_ptr<Powiter::Cache<Powiter::Row> >  _nodeCache;
    
    boost::scoped_ptr<Powiter::Cache<Powiter::FrameEntry> > _viewerCache;
    
};

namespace Powiter{
    
    inline void errorDialog(const std::string& title,const std::string& message){
        appPTR->getTopLevelInstance()->errorDialog(title,message);
    }
    
    inline void warningDialog(const std::string& title,const std::string& message){
        appPTR->getTopLevelInstance()->warningDialog(title,message);
    }
    
    inline void informationDialog(const std::string& title,const std::string& message){
        appPTR->getTopLevelInstance()->informationDialog(title,message);
    }
    
    inline Powiter::StandardButton questionDialog(const std::string& title,const std::string& message,Powiter::StandardButtons buttons =
                                           Powiter::StandardButtons(Powiter::Yes | Powiter::No),
                                                  Powiter::StandardButton defaultButton = Powiter::NoButton){
        return appPTR->getTopLevelInstance()->questionDialog(title,message,buttons,defaultButton);
    }
}


#endif // POWITER_GLOBAL_CONTROLER_H_

